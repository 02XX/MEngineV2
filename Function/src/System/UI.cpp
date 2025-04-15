#include "System/UI.hpp"

namespace MEngine
{
UI::UI(std::shared_ptr<ILogger> logger, std::shared_ptr<Context> context, std::shared_ptr<IWindow> window,
       std::shared_ptr<RenderPassManager> renderPassManager, std::shared_ptr<ImageFactory> imageFactory,
       std::shared_ptr<CommandBufferManager> commandBufferManager,
       std::shared_ptr<SyncPrimitiveManager> syncPrimitiveManager, std::shared_ptr<SamplerManager> samplerManager,
       std::shared_ptr<entt::registry> registry, std::shared_ptr<IRepository<PBRMaterial>> pbrMaterialRepository,
       std::shared_ptr<IRepository<Texture>> textureRepository)
    : mLogger(logger), mContext(context), mWindow(window), mRenderPassManager(renderPassManager),
      mImageFactory(imageFactory), mCommandBufferManager(commandBufferManager),
      mSyncPrimitiveManager(syncPrimitiveManager), mSamplerManager(samplerManager), mRegistry(registry),
      mPBRMaterialRepository(pbrMaterialRepository), mTextureRepository(textureRepository)
{
    mIconTransitionCommandBuffer = mCommandBufferManager->CreatePrimaryCommandBuffer(CommandBufferType::Graphic);
    mIconSampler = mSamplerManager->CreateUniqueSampler(vk::Filter::eLinear, vk::Filter::eLinear);
    mSceneSampler = mSamplerManager->CreateUniqueSampler(vk::Filter::eLinear, vk::Filter::eLinear);
    mIconTransitionFence = mSyncPrimitiveManager->CreateFence();
    //  Initialize ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    mIO = &ImGui::GetIO();
    mIO->ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard | ImGuiConfigFlags_DockingEnable;
    ImGui::StyleColorsDark();
    // Initialize ImGui Vulkan Backend
    ImGui_ImplSDL3_InitForVulkan(static_cast<SDL_Window *>(mWindow->GetNativeHandle()));
    ImGui_ImplVulkan_InitInfo initInfo{};
    initInfo.ApiVersion = mContext->GetInstanceVersion();
    initInfo.Instance = mContext->GetInstance();
    initInfo.PhysicalDevice = mContext->GetPhysicalDevice();
    initInfo.Device = mContext->GetDevice();
    initInfo.QueueFamily = mContext->GetQueueFamilyIndicates().graphicsFamily.value();
    initInfo.Queue = mContext->GetGraphicQueue();
    initInfo.RenderPass = mRenderPassManager->GetRenderPass(RenderPassType::UI);
    initInfo.MinImageCount = mContext->GetSurfaceInfo().imageCount;
    initInfo.ImageCount = mContext->GetSurfaceInfo().imageCount;
    initInfo.MSAASamples = static_cast<VkSampleCountFlagBits>(vk::SampleCountFlagBits::e1);
    // optional
    initInfo.Subpass = 0;
    initInfo.DescriptorPoolSize = 1000;
    ImGui_ImplVulkan_Init(&initInfo);

    // Upload Fonts
    mIO->Fonts->AddFontDefault();
    mMSYHFont = mIO->Fonts->AddFontFromFileTTF((mUIResourcePath / "Font" / "MSYH.TTC").string().c_str(), 16.0f, nullptr,
                                               mIO->Fonts->GetGlyphRangesChineseSimplifiedCommon());
    mIO->FontDefault = mMSYHFont;
    mAssetRegistry = std::make_shared<entt::registry>();
    mAssetRegistry->on_construct<AssetsComponent>().connect<&UI::OnAssetCreated>(this);
    mAssetRegistry->on_update<AssetsComponent>().connect<&UI::OnAssetUpdated>(this);
    mAssetRegistry->on_destroy<AssetsComponent>().connect<&UI::OnAssetDestroyed>(this);
    // ImGuizmo
    ImGuizmo::SetImGuiContext(ImGui::GetCurrentContext());
    ImGuizmo::Enable(true);

    // Icons
    LoadUIIcon(mUIResourcePath / "Icon" / "folder.png", mFolderIcon);
    LoadUIIcon(mUIResourcePath / "Icon" / "file.png", mFileIcon);
    // auto defaultTexture = mTextureManager->GetDefaultTexture();
    // mDefaultTextureDescriptorSet =
    //     ImGui_ImplVulkan_AddTexture(mSceneSampler.get(), defaultTexture->GetImageView(),
    //                                 static_cast<VkImageLayout>(vk::ImageLayout::eShaderReadOnlyOptimal));
    SetSceneViewPort();

    EntryFolder(mProjectPath);
}
void UI::ProcessEvent(const SDL_Event *event)
{
    ImGui_ImplSDL3_ProcessEvent(event);
}
void UI::SetSceneViewPort()
{
    std::vector<vk::ImageView> imageViews;
    for (auto imageView : mRenderPassManager->GetTransparentFrameResource())
    {
        imageViews.push_back(imageView->renderTargetImageView.get());
    }
    for (auto sceneDescriptorSet : mSceneDescriptorSets)
    {
        ImGui_ImplVulkan_RemoveTexture(sceneDescriptorSet);
    }
    mSceneDescriptorSets.clear();
    for (size_t i = 0; i < imageViews.size(); ++i)
    {
        mSceneDescriptorSets.push_back(ImGui_ImplVulkan_AddTexture(
            mSceneSampler.get(), imageViews[i], static_cast<VkImageLayout>(vk::ImageLayout::eShaderReadOnlyOptimal)));
    }
}
void UI::CollectEntity()
{
    // camera
    auto entities = mRegistry->view<CameraComponent>();
    for (auto entity : entities)
    {
        auto &camera = entities.get<CameraComponent>(entity);
        if (camera.isMainCamera)
        {
            mMainCamera = entity;
            break;
        }
    }
}

void UI::LoadUIIcon(const std::filesystem::path &iconPath, vk::DescriptorSet &descriptorSet)
{
    mIconTransitionCommandBuffer->reset();
    int iconWidth, iconHeight, iconChannels;
    stbi_set_flip_vertically_on_load(true);
    auto iconData = stbi_load(iconPath.string().c_str(), &iconWidth, &iconHeight, &iconChannels, STBI_rgb_alpha);
    iconChannels = 4;
    if (iconData)
    {
        // 1. 创建Image
        vk::DeviceSize folderSize = iconWidth * iconHeight * iconChannels;
        mIconImages.push_back(mImageFactory->CreateImage(ImageType::Texture2D, vk::Extent3D(iconWidth, iconHeight, 1),
                                                         folderSize, iconData));
        // 2. 创建ImageView
        auto imageView = mImageFactory->CreateImageView(mIconImages.back().get());
        mIconImageViews.push_back(std::move(imageView));
        // 3. 转换布局
        vk::ImageMemoryBarrier imagebarrier;
        imagebarrier.setImage(mIconImages.back()->GetHandle())
            .setOldLayout(vk::ImageLayout::eUndefined)
            .setNewLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
            .setSrcQueueFamilyIndex(mContext->GetQueueFamilyIndicates().graphicsFamily.value())
            .setDstQueueFamilyIndex(mContext->GetQueueFamilyIndicates().graphicsFamily.value())
            .setSubresourceRange(vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1))
            .setSrcAccessMask(vk::AccessFlagBits::eNoneKHR)
            .setDstAccessMask(vk::AccessFlagBits::eShaderRead);
        mIconTransitionCommandBuffer->begin(vk::CommandBufferBeginInfo());
        mIconTransitionCommandBuffer->pipelineBarrier(vk::PipelineStageFlagBits::eTopOfPipe,
                                                      vk::PipelineStageFlagBits::eFragmentShader, {}, {}, {},
                                                      imagebarrier);
        mIconTransitionCommandBuffer->end();
        vk::SubmitInfo submitInfo;
        submitInfo.setCommandBuffers(mIconTransitionCommandBuffer.get());
        //  4. 创建描述符集
        descriptorSet =
            ImGui_ImplVulkan_AddTexture(mIconSampler.get(), mIconImageViews.back().get(),
                                        static_cast<VkImageLayout>(vk::ImageLayout::eShaderReadOnlyOptimal));
        mContext->GetDevice().resetFences({mIconTransitionFence.get()});
        mContext->SubmitToGraphicQueue({submitInfo}, mIconTransitionFence.get());
        auto result = mContext->GetDevice().waitForFences({mIconTransitionFence.get()}, VK_TRUE, 1000000000); // 1s
        if (result != vk::Result::eSuccess)
        {
            mLogger->Error("Failed to create icon: {}", iconPath.string());
        }
        mLogger->Debug("loaded icon: {}", iconPath.string());
    }
    else
    {
        mLogger->Error("Failed to load icon: {}", iconPath.string());
    }
    stbi_image_free(iconData);
}
void UI::SetDefaultWindowLayout()
{
    ImGui::DockBuilderRemoveNode(mDockSpaceID);
    ImGui::DockBuilderAddNode(mDockSpaceID, ImGuiDockNodeFlags_DockSpace);
    ImGui::DockBuilderSetNodeSize(mDockSpaceID, mMainViewport->WorkSize);
    // 1. 主区域拆分为底部（30%）和顶部（70%）
    ImGuiID dockBottomID, dockTopID;
    ImGui::DockBuilderSplitNode(mDockSpaceID, ImGuiDir_Down, 0.3, &dockBottomID, &dockTopID);
    // 2. 顶部区域拆分为左（30%）和剩余部分（70%）
    ImGuiID dockLeftID, remainingTop;
    ImGui::DockBuilderSplitNode(dockTopID, ImGuiDir_Left, 0.3, &dockLeftID, &remainingTop);
    // 3. 剩余部分（70%）拆分为中（60%）和右（40%）
    ImGuiID dockCenterID, dockRightID;
    ImGui::DockBuilderSplitNode(remainingTop, ImGuiDir_Right, 0.4, &dockRightID, &dockCenterID);
    // 4. 中部拆分上(20%)和下(80%)
    ImGuiID dockTopCenterID, dockBottomCenterID;
    ImGui::DockBuilderSplitNode(dockCenterID, ImGuiDir_Up, 0.15, &dockTopCenterID, &dockBottomCenterID);
    // 绑定窗口
    ImGui::DockBuilderDockWindow("SceneView", dockBottomCenterID); // 中间
    ImGui::DockBuilderDockWindow("Hierarchy", dockLeftID);         // 左侧
    ImGui::DockBuilderDockWindow("Inspector", dockRightID);        // 右侧
    ImGui::DockBuilderDockWindow("Assets", dockBottomID);          // 底部
    ImGui::DockBuilderDockWindow("Toolbar", dockTopCenterID);      // 顶部
    ImGui::DockBuilderFinish(mDockSpaceID);
}
void UI::RightClickMenu()
{
}
void UI::DockingSpace()
{
    mMainViewport = ImGui::GetMainViewport();
    mDockSpaceID = ImGui::DockSpaceOverViewport();
}

void UI::HierarchyWindow()
{
    ImGui::Begin("Hierarchy", nullptr, ImGuiWindowFlags_None);
    if (ImGui::IsWindowFocused())
    {
        mAssetsHoveredEntity = entt::null;
        mAssetsSelectedEntity = entt::null;
    }
    // 显示实体列表
    auto entities = mRegistry->view<TransformComponent>();
    for (auto entity : entities)
    {
        ImGui::PushID(static_cast<int>(entt::to_integral(entity)));
        auto entityLabel = std::format("Entity {}", entt::to_integral(entity));
        if (ImGui::Selectable(entityLabel.c_str(), mSelectedEntity == entity))
        {
            mSelectedEntity = entity;
        }
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenBlockedByPopup))
        {
            mHoveredEntity = entity;
            if (ImGui::IsMouseClicked(ImGuiMouseButton_Right))
            {
                mSelectedEntity = entity;
            }
        }
        ImGui::PopID();
    }
    if (ImGui::IsWindowHovered() && !ImGui::IsAnyItemHovered())
    {
        mHoveredEntity = entt::null;
    }
    // 右键菜单
    if (ImGui::IsMouseClicked(ImGuiMouseButton_Right) && ImGui::IsWindowHovered())
    {
        ImGui::OpenPopup("HierarchyContextMenu");
    }
    if (ImGui::BeginPopupContextWindow("HierarchyContextMenu", ImGuiPopupFlags_MouseButtonRight))
    {
        if (ImGui::MenuItem("Create Entity"))
        {
            auto entity = mRegistry->create();
            mRegistry->emplace<TransformComponent>(entity);
            mSelectedEntity = entity;
        }
        if (mHoveredEntity != entt::null)
        {
            ImGui::Separator();
            if (ImGui::MenuItem("Delete Entity"))
            {
                mContext->GetDevice().waitIdle();
                mRegistry->destroy(mSelectedEntity);
                mSelectedEntity = entt::null;
                mHoveredEntity = entt::null;
            }
        }
        ImGui::EndPopup();
    }
    ImGui::End();
}
void UI::ShowTexture(const std::string &name, std::shared_ptr<Texture> texture, ImVec2 size)
{

    ImGui::Columns(2, nullptr, false); // 创建两列布局，不显示分隔线
    ImGui::SetColumnWidth(0, 100);     // 固定文本列宽
    ImGui::Text("%s", name.c_str());
    ImGui::NextColumn(); // 切换到图片列
    if (texture == nullptr)
    {
        ImGui::Image(reinterpret_cast<ImTextureID>(static_cast<VkDescriptorSet>(mDefaultTextureDescriptorSet)), size,
                     ImVec2(0, 1), ImVec2(1, 0));
    }
    else
    {
        // if (!texture->GetUIDescriptorID())
        // {
        //     auto id = ImGui_ImplVulkan_AddTexture(texture->GetSampler(), texture->GetImageView(),
        //                                           static_cast<VkImageLayout>(vk::ImageLayout::eShaderReadOnlyOptimal));
        //     texture->SetUIDescriptorID(id);
        // }
        // ImGui::Image(reinterpret_cast<ImTextureID>(static_cast<VkDescriptorSet>(texture->GetUIDescriptorID())), size,
        //              ImVec2(0, 1), ImVec2(1, 0));
    }
    ImGui::Dummy(ImVec2(0, 5));

    ImGui::Columns(1); // 恢复单列布局
}
void UI::InspectorMaterial(MaterialComponent &materialComponent)
{
    if (ImGui::CollapsingHeader("Material Attribute", ImGuiTreeNodeFlags_DefaultOpen))
    {
        auto &material = materialComponent.material;
        auto pipelineTypeNames = magic_enum::enum_names<PipelineType>();
        // if (ImGui::BeginCombo("Pipeline Type", magic_enum::enum_name(material->GetPipelineType()).data()))
        // {
        //     for (int i = 0; i < pipelineTypeNames.size(); i++)
        //     {
        //         bool isSelected = (i == static_cast<int>(material->GetPipelineType()));
        //         if (ImGui::Selectable(pipelineTypeNames[i].data(), isSelected))
        //         {
        //             material->SetPipelineType(static_cast<PipelineType>(i));
        //             mMaterialManager->SaveMaterial(material->GetMaterialPath(), material);
        //         }
        //     }
        //     ImGui::EndCombo();
        // }
        // switch (material->GetPipelineType())
        // {
        // case PipelineType::ShadowMap:
        // case PipelineType::ForwardOpaque:
        // case PipelineType::ForwardTransparent: {
        //     auto pbrMaterial = std::static_pointer_cast<PBRMaterial>(material);
        //     auto pbrMaterialTextures = pbrMaterial->GetMaterialTextures();
        //     auto pbrMaterialParams = pbrMaterial->GetMaterialParams();
        //     auto pbrTextureFlag = pbrMaterial->GetMaterialTextureFlag();
        //     auto albedoColor = glm::value_ptr(pbrMaterialParams.parameters.albedo);
        //     if (ImGui::ColorEdit3("Albedo", albedoColor))
        //     {
        //         pbrMaterialParams.parameters.albedo = glm::vec3(albedoColor[0], albedoColor[1], albedoColor[2]);
        //         pbrMaterial->SetPBRParameters(pbrMaterialParams.parameters);
        //         mContext->GetDevice().waitIdle();
        //         pbrMaterial->Update();
        //         mMaterialManager->SaveMaterial(pbrMaterial->GetMaterialPath(), pbrMaterial);
        //     }
        //     ImGui::Dummy(ImVec2(0, 5));
        //     ShowTexture("Albedo Map", pbrTextureFlag.useAlbedoMap ? pbrMaterialTextures.albedoMap : nullptr);
        //     ShowTexture("Normal Map", pbrTextureFlag.useNormalMap ? pbrMaterialTextures.normalMap : nullptr);
        //     ShowTexture("MetallicRoughness Map",
        //                 pbrTextureFlag.useMetallicRoughnessMap ? pbrMaterialTextures.metallicRoughnessMap : nullptr);
        //     ShowTexture("AO Map", pbrTextureFlag.useAOMap ? pbrMaterialTextures.aoMap : nullptr);
        //     ShowTexture("Emissive Map", pbrTextureFlag.useEmissiveMap ? pbrMaterialTextures.emissiveMap : nullptr);
        //     break;
        // }
        // case PipelineType::DeferredGBuffer:
        // case PipelineType::DeferredLighting:
        // case PipelineType::ScreenSpaceEffectSSAO:
        // case PipelineType::ScreenSpaceEffectSSR:
        // case PipelineType::SkinnedMesh:
        // case PipelineType::MorphTarget:
        // case PipelineType::ParticleCPU:
        // case PipelineType::ParticleGPU:
        // case PipelineType::Decal:
        // case PipelineType::PostProcessToneMapping:
        // case PipelineType::PostProcessBloom:
        // case PipelineType::PostProcessDOF:
        // case PipelineType::PostProcessMotionBlur:
        // case PipelineType::PostProcessFXAA:
        // case PipelineType::PostProcessSMAA:
        // case PipelineType::PostProcessVignette:
        // case PipelineType::PostProcessChromaticAberration:
        // case PipelineType::PostProcessFilmGrain:
        // case PipelineType::PostProcessColorGrading:
        // case PipelineType::UISprite:
        // case PipelineType::UIText:
        // case PipelineType::Toon:
        // case PipelineType::Wireframe:
        //     break;
        // };
    }
}
void UI::InspectorWindow()
{
    ImGui::Begin("Inspector", nullptr, ImGuiWindowFlags_None);
    // 1. 显示选中实体的属性
    if (mSelectedEntity != entt::null)
    {
        auto entityLabel = std::format("Entity {}", static_cast<uint32_t>(mSelectedEntity));
        ImGui::Text("%s", entityLabel.c_str());
        // 显示TransomComponent
        if (mRegistry->any_of<TransformComponent>(mSelectedEntity))
        {
            if (ImGui::CollapsingHeader("Transform Component", ImGuiTreeNodeFlags_DefaultOpen))
            {
                ImGui::BeginChild("Transform", ImVec2(0, 100), true);
                auto &transform = mRegistry->get<TransformComponent>(mSelectedEntity);
                auto modelMatrix = transform.modelMatrix;
                float matrixTranslation[3], matrixRotation[3], matrixScale[3];
                ImGuizmo::DecomposeMatrixToComponents(glm::value_ptr(modelMatrix), matrixTranslation, matrixRotation,
                                                      matrixScale);
                ImGui::DragFloat3("Position", matrixTranslation, 0.1f);
                ImGui::DragFloat3("Rotation", matrixRotation, 0.1f);
                ImGui::DragFloat3("Scale", matrixScale, 0.1f);
                ImGuizmo::RecomposeMatrixFromComponents(matrixTranslation, matrixRotation, matrixScale,
                                                        glm::value_ptr(modelMatrix));
                // 分解矩阵
                glm::vec3 translation, scale, skew;
                glm::quat rotation;
                glm::vec4 perspective;
                if (glm::decompose(modelMatrix, scale, rotation, translation, skew, perspective))
                {
                    transform.position = translation;
                    transform.rotation = rotation;
                    transform.scale = scale;
                }
                ImGui::EndChild();
            }
        }
        // 显示MeshComponent
        if (mRegistry->any_of<MeshComponent>(mSelectedEntity))
        {
        }
        // 显示MaterialComponent
        if (mRegistry->any_of<MaterialComponent>(mSelectedEntity))
        {
            auto &material = mRegistry->get<MaterialComponent>(mSelectedEntity);
            if (ImGui::CollapsingHeader("Material Component", ImGuiTreeNodeFlags_DefaultOpen))
            {
                // 获取所有材质
                // auto materials = mMaterialManager->GetAllMaterials();
                // std::string name =
                // material.material->GetMaterialName() + "@" + std::to_string(material.material->GetMaterialID());
                // 显示材质列表
                // if (ImGui::BeginCombo("Material", name.c_str()))
                // {
                //     for (int i = 0; i < materials.size(); i++)
                //     {
                //         ImGui::PushID(i);
                //         std::string materialName =
                //             materials[i]->GetMaterialName() + "@" + std::to_string(materials[i]->GetMaterialID());
                //         bool isSelected = (materials[i]->GetMaterialID() == material.material->GetMaterialID());
                //         if (ImGui::Selectable(materialName.c_str(), isSelected))
                //         {
                //             material.material = materials[i];
                //         }
                //         ImGui::PopID();
                //     }
                //     ImGui::EndCombo();
                // }
            }
            InspectorMaterial(material);
        }
    }
    // 2. 显示选中资源的属性
    if (mAssetsSelectedEntity != entt::null)
    {
        auto entityLabel = std::format("Asset {}", static_cast<uint32_t>(mAssetsSelectedEntity));
        ImGui::Text("%s", entityLabel.c_str());
        // 显示AssetsComponent
        if (mAssetRegistry->any_of<AssetsComponent>(mAssetsSelectedEntity))
        {
            if (ImGui::CollapsingHeader("Assets Component", ImGuiTreeNodeFlags_DefaultOpen))
            {
                auto &asset = mAssetRegistry->get<AssetsComponent>(mAssetsSelectedEntity);
                ImGui::Text("Path: %s", asset.path.string().c_str());
                ImGui::Text("Name: %s", asset.name.c_str());
                ImGui::Text("Type: %s", magic_enum::enum_name(asset.type).data());
            }
        }
        // 显示MaterialComponent
        if (mAssetRegistry->any_of<MaterialComponent>(mAssetsSelectedEntity))
        {
            auto &material = mAssetRegistry->get<MaterialComponent>(mAssetsSelectedEntity);
            InspectorMaterial(material);
        }
    }
    ImGui::End();
}
void UI::ToolbarWindow()
{
    ImGui::Begin("Toolbar", nullptr, ImGuiWindowFlags_None);
    // 显示工具栏按钮
    ImGui::BeginGroup();
    {
        if (ImGui::Button("Play"))
        {
            mIsPlay = !mIsPlay;
        }
        ImGui::SameLine();
        if (ImGui::Button("Pause"))
        {
            mIsPause = !mIsPause;
        }
        ImGui::SameLine();
        if (ImGui::Button("Stop"))
        {
            mIsStop = !mIsStop;
        }
    }
    ImGui::EndGroup();
    ImGui::SameLine();
    // 显示窗口标题 FPS 等信息
    ImGui::BeginGroup();
    {
        auto sceneWindow = ImGui::FindWindowByName("SceneView");
        ImGui::Text("SceneView Size: %1.f x %1.f", sceneWindow->ContentSize.x, sceneWindow->ContentSize.y);
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(1, 1, 0, 1), "FPS: %1.f", ImGui::GetIO().Framerate);
        if (ImGui::RadioButton("Translate", mGuizmoOperation == ImGuizmo::TRANSLATE) || ImGui::IsKeyDown(ImGuiKey_W))
            mGuizmoOperation = ImGuizmo::TRANSLATE;
        ImGui::SameLine();
        if (ImGui::RadioButton("Rotate", mGuizmoOperation == ImGuizmo::ROTATE) || ImGui::IsKeyDown(ImGuiKey_E))
            mGuizmoOperation = ImGuizmo::ROTATE;
        ImGui::SameLine();
        if (ImGui::RadioButton("Scale", mGuizmoOperation == ImGuizmo::SCALE) || ImGui::IsKeyDown(ImGuiKey_R))
            mGuizmoOperation = ImGuizmo::SCALE;
        if (ImGui::RadioButton("Local", mGuizmoMode == ImGuizmo::LOCAL))
            mGuizmoMode = ImGuizmo::LOCAL;
        ImGui::SameLine();
        if (ImGui::RadioButton("World", mGuizmoMode == ImGuizmo::WORLD))
            mGuizmoMode = ImGuizmo::WORLD;
    }
    ImGui::EndGroup();
    ImGui::End();
}
void UI::SceneViewWindow()
{
    ImGui::Begin("SceneView", nullptr, ImGuiWindowFlags_None);
    ImVec2 windowPos = ImGui::GetWindowPos();
    ImVec2 windowSize = ImGui::GetContentRegionAvail();
    // 尺寸变化检测
    uint32_t width = static_cast<uint32_t>(windowSize.x);
    uint32_t height = static_cast<uint32_t>(windowSize.y);
    mIsSceneViewPortChanged = false;
    // 获取Camera
    auto &camera = mRegistry->get<CameraComponent>(mMainCamera);
    camera.aspectRatio = static_cast<float>(width) / static_cast<float>(height);
    if (width != mSceneViewPortWidth || height != mSceneViewPortHeight)
    {
        mSceneViewPortWidth = width;
        mSceneViewPortHeight = height;
        // mContext->GetDevice().waitIdle();
        // mRenderPassManager->RecreateFrameBuffer(width, height);
        // SetSceneViewPort();
    }
    else
    {
        // 显示场景视图
        ImTextureID textureId =
            reinterpret_cast<ImTextureID>(static_cast<VkDescriptorSet>(mSceneDescriptorSets[mImageIndex]));
        ImGui::Image(textureId, ImVec2(mSceneViewPortWidth, mSceneViewPortHeight), ImVec2(0, 1), ImVec2(1, 0));

        // 3. imGuizmo
        ImVec2 imagePos = ImGui::GetItemRectMin();
        ImVec2 imageSize = ImGui::GetItemRectSize();
        ImGuizmo::SetRect(imagePos.x, imagePos.y, imageSize.x, imageSize.y);
        ImGuizmo::SetDrawlist(ImGui::GetCurrentWindow()->DrawList);
        if (mSelectedEntity != entt::null)
        {
            auto &transform = mRegistry->get<TransformComponent>(mSelectedEntity);
            auto modelMatrix = transform.modelMatrix;
            ImGuizmo::Manipulate(glm::value_ptr(camera.viewMatrix), glm::value_ptr(camera.projectionMatrix),
                                 mGuizmoOperation, mGuizmoMode, glm::value_ptr(modelMatrix));
            if (ImGuizmo::IsUsing())
            {
                // 分解矩阵
                glm::vec3 translation, scale, skew;
                glm::quat rotation;
                glm::vec4 perspective;
                if (glm::decompose(modelMatrix, scale, rotation, translation, skew, perspective))
                {
                    transform.position = translation;
                    transform.rotation = rotation;
                    transform.scale = scale;
                }
            }
        }
    }
    ImGui::End();
}
void UI::AssetWindow()
{
    ImGui::Begin("Assets", nullptr, ImGuiWindowFlags_MenuBar);
    if (ImGui::IsWindowFocused())
    {
        mHoveredEntity = entt::null;
        mSelectedEntity = entt::null;
    }
    // 1. 菜单栏
    if (ImGui::BeginMenuBar())
    {
        if (ImGui::Button("<-"))
        {
            if (mCurrentPath != mProjectPath)
            {
                EntryFolder(mCurrentPath.parent_path());
            }
        }
        ImGui::SameLine();
        ImGui::Text("%s", mCurrentPath.string().c_str());
        ImGui::EndMenuBar();
    }
    ImGui::Separator();
    DisplayAssetEntity();
    // 4. 右键菜单
    if (ImGui::IsMouseClicked(ImGuiMouseButton_Right) && ImGui::IsWindowHovered())
    {
        ImGui::OpenPopup("AssetContextMenu");
    }
    if (ImGui::BeginPopupContextWindow("AssetContextMenu", ImGuiPopupFlags_MouseButtonRight))
    {
        if (ImGui::MenuItem("Create Folder"))
        {
            std::string folderName = "NewFolder";
            std::filesystem::path newFolderPath = mCurrentPath / folderName;
            int count = 0;
            while (std::filesystem::exists(newFolderPath))
            {
                folderName = std::format("NewFolder{}", ++count);
                newFolderPath = mCurrentPath / folderName;
            }
            std::filesystem::create_directory(newFolderPath);
            UpdateAssetsView();
        }
        if (ImGui::MenuItem("Create Material"))
        {
            std::string materialName = "NewMaterial.mat";
            int count = 0;
            std::filesystem::path newMaterialPath = mCurrentPath / materialName;
            while (std::filesystem::exists(newMaterialPath))
            {
                materialName = std::format("NewMaterial{}.mat", ++count);
                newMaterialPath = mCurrentPath / materialName;
            }
            // mMaterialManager->CreateMaterial(newMaterialPath);
            UpdateAssetsView();
        }
        if (ImGui::MenuItem("Delete"))
        {
        }
        ImGui::EndPopup();
    }
    ImGui::End();
}
void UI::DisplayAssetEntity()
{
    if (!std::filesystem::exists(mCurrentPath))
    {
        ImGui::TextColored(ImVec4(1, 0, 0, 1), "Directory not exists!");
        ImGui::End();
        return;
    }
    try
    {
        // ImGuiListClipper clipper;
        // clipper.Begin(mProcessedAssets.size());
        // while (clipper.Step())
        // {
        const int itemWidth = mIconSize + 10;
        const int itemHeight = mIconSize + 20;
        const int columns = std::max(1, static_cast<int>(ImGui::GetContentRegionAvail().x / itemWidth));
        ImGui::Columns(columns, "AssetColumns", false); // false = 不显示边框
        for (int i = 0; i < mProcessedAssets.size(); ++i)
        {
            auto entity = mProcessedAssets[i];
            auto &asset = mAssetRegistry->get<AssetsComponent>(entity);

            ImGui::PushID(static_cast<int>(entt::to_integral(entity)));

            // 绘制图标按钮
            const bool isSelected = (mAssetsSelectedEntity == entity);
            if (ImGui::Selectable("##btn", isSelected,
                                  ImGuiSelectableFlags_AllowItemOverlap | ImGuiSelectableFlags_AllowDoubleClick,
                                  ImVec2(itemWidth, itemHeight)))
            {
                mAssetsSelectedEntity = entity;
                // 处理双击
                if (ImGui::IsMouseDoubleClicked(0))
                {
                    if (asset.type == AssetType::Folder)
                        EntryFolder(mCurrentPath / asset.name);
                }
            }
            if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenBlockedByPopup))
            {
                mAssetsHoveredEntity = entity;
                if (ImGui::IsMouseClicked(ImGuiMouseButton_Right))
                    mAssetsSelectedEntity = entity;
                if (ImGui::IsMouseDragging(0))
                {
                    if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
                    {
                        // 设置拖拽数据
                        ImGui::SetDragDropPayload("ASSET_ITEM", &entity, sizeof(entity));
                        // 显示拖拽预览（图标）
                        ImGui::Image(
                            reinterpret_cast<ImTextureID>(static_cast<VkDescriptorSet>(asset.iconDescriptorSet)),
                            ImVec2(mIconSize, mIconSize), ImVec2(0, 1), ImVec2(1, 0));
                        // 可选：添加文字说明
                        ImGui::Text("拖动 %s", asset.name.c_str());
                        ImGui::EndDragDropSource();
                    }
                }
            }
            ImVec2 ContainerMinPos = ImGui::GetItemRectMin();
            ImVec2 ContainerMaxPos = ImGui::GetItemRectMax();
            ImVec2 ContainerSize = ImGui::GetItemRectSize();
            // 绘制图标
            ImVec2 iconPos = ImVec2(ContainerMinPos.x, ContainerMinPos.y);
            ImGui::SetCursorScreenPos(iconPos);
            ImGui::Image(reinterpret_cast<ImTextureID>(static_cast<VkDescriptorSet>(asset.iconDescriptorSet)),
                         ImVec2(mIconSize, mIconSize), ImVec2(0, 1), ImVec2(1, 0));
            // 绘制文本
            ImVec2 textPos =
                ImVec2(ContainerMinPos.x + (ContainerSize.x - mIconSize) / 2, ContainerMinPos.y + mIconSize);
            ImGui::SetCursorScreenPos(textPos);
            ImGui::Text("%s", asset.name.c_str());

            ImGui::PopID();
            ImGui::NextColumn(); // 移动到下一列
        }
        ImGui::Columns(1); // 恢复单列模式
        // }
        // clipper.End();
    }
    catch (const std::exception &e)
    {
        ImGui::TextColored(ImVec4(1, 0, 0, 1), "Error: %s", e.what());
    }
}
void UI::RenderUI()
{
    CollectEntity();
    ImGui_ImplSDL3_NewFrame();
    ImGui_ImplVulkan_NewFrame();
    ImGui::NewFrame();
    ImGuizmo::BeginFrame();
    DockingSpace();
    if (mIsFirstFrame)
    {
        SetDefaultWindowLayout();
        mIsFirstFrame = false;
    }
    RightClickMenu();
    HierarchyWindow();
    InspectorWindow();
    AssetWindow();
    SceneViewWindow();
    ToolbarWindow();
}
void UI::RecordUICommandBuffer(vk::CommandBuffer commandBuffer)
{
    ImGui::Render();
    ImDrawData *drawData = ImGui::GetDrawData();
    bool isMinimized = (drawData->DisplaySize.x <= 0.0f || drawData->DisplaySize.y <= 0.0f);
    if (!isMinimized)
    {
        ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer);
    }
}
UI::~UI()
{
    mIO->Fonts->Clear();
    mIO->Fonts->ClearInputData();
    mIO->Fonts->ClearTexData();
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();
    mLogger->Info("UI System Shutdown");
}

void UI::OnAssetCreated(entt::registry &, entt::entity entity)
{
    mProcessedAssets.push_back(entity);
    ProcessAssets();
}
void UI::OnAssetUpdated(entt::registry &, entt::entity)
{
}
void UI::OnAssetDestroyed(entt::registry &, entt::entity entity)
{
    auto it = std::remove(mProcessedAssets.begin(), mProcessedAssets.end(), entity);
    if (it != mProcessedAssets.end())
    {
        mProcessedAssets.erase(it, mProcessedAssets.end());
    }
}
void UI::UpdateAssetsView()
{
    if (std::filesystem::exists(mCurrentPath))
    {
        mAssetRegistry->clear();
        for (const auto &entry : std::filesystem::directory_iterator(mCurrentPath))
        {
            if (entry.is_directory())
            {
                auto entity = mAssetRegistry->create();
                mAssetRegistry->emplace<AssetsComponent>(entity, entry.path(), entry.path().filename().string(),
                                                         AssetType::Folder, mFolderIcon);
            }
            else
            {
                auto entity = mAssetRegistry->create();

                // material
                if (entry.path().extension() == ".mat")
                {
                    mAssetRegistry->emplace<AssetsComponent>(entity, entry.path(), entry.path().filename().string(),
                                                             AssetType::Material, mFileIcon);
                    // auto materialID = mMaterialManager->LoadMaterialFromFile(entry.path().string());
                    // auto material = mMaterialManager->GetMaterial(materialID);
                    // mAssetRegistry->emplace<MaterialComponent>(entity, material);
                }
                else if (entry.path().extension() == ".tex")
                {
                    mAssetRegistry->emplace<AssetsComponent>(entity, entry.path(), entry.path().filename().string(),
                                                             AssetType::Texture, mFileIcon);
                }
                else if (entry.path().extension() == ".anim")
                {
                    mAssetRegistry->emplace<AssetsComponent>(entity, entry.path(), entry.path().filename().string(),
                                                             AssetType::Animation, mFileIcon);
                }
                else if (entry.path().extension() == ".fbx")
                {
                    mAssetRegistry->emplace<AssetsComponent>(entity, entry.path(), entry.path().filename().string(),
                                                             AssetType::Model, mFileIcon);
                }
                else
                {
                    mAssetRegistry->emplace<AssetsComponent>(entity, entry.path(), entry.path().filename().string(),
                                                             AssetType::File, mFileIcon);
                }
            }
        }
    }
    else
    {
        mLogger->Error("Path does not exist: {}", mCurrentPath.string());
    }
}
void UI::ProcessAssets()
{
    std::sort(mProcessedAssets.begin(), mProcessedAssets.end(), [this](entt::entity a, entt::entity b) {
        auto &compA = mAssetRegistry->get<AssetsComponent>(a);
        auto &compB = mAssetRegistry->get<AssetsComponent>(b);
        return (compA.type == AssetType::Folder) > (compB.type == AssetType::Folder) ||
               (compA.type == compB.type && compA.name < compB.name);
    });
}
void UI::EntryFolder(const std::filesystem::path &path)
{
    mCurrentPath = path;
    UpdateAssetsView();
}
} // namespace MEngine