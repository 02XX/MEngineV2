
#include "nlohmann/json.hpp"
#include "nlohmann/adl_serializer.hpp"
#include "gtest/gtest.h"
#include <memory>
class Entity
{
  public:
    std::string name;
    int id;
};
namespace nlohmann
{
template <> class adl_serializer<std::shared_ptr<Entity>>
{
  public:
    static void to_json(json &j, const std::shared_ptr<Entity> &entity)
    {
        j["name"] = entity->name;
        j["id"] = entity->id;
    }

    static void from_json(const json &j, std::shared_ptr<Entity> &entity)
    {
        entity = std::make_shared<Entity>();
        j.at("name").get_to(entity->name);
        j.at("id").get_to(entity->id);
    }
};

} // namespace nlohmann
TEST(JsonTest, Serialize)
{
    std::shared_ptr<Entity> entity = std::make_shared<Entity>();
    entity->name = "TestEntity";
    entity->id = 1;

    // Serialize to JSON
    nlohmann::json j;
    j = entity;
    std::string jsonString = j.dump();
    // 是否含有TestEntity
    EXPECT_NE(jsonString.find("TestEntity"), std::string::npos);
    std::cout << "Serialized JSON: " << jsonString << std::endl;
}
TEST(JsonTest, Deserialize)
{
    std::string jsonString = R"({"name":"TestEntity","id":1})";
    std::cout << "Deserializing JSON: " << jsonString << std::endl;
    nlohmann::json j = nlohmann::json::parse(jsonString);

    // Deserialize from JSON
    std::shared_ptr<Entity> entity = j;
    // j.get_to(entity);

    EXPECT_EQ(entity->name, "TestEntity");
    EXPECT_EQ(entity->id, 1);
}