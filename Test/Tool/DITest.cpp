#include "boost/di.hpp"
#include "gtest/gtest.h"
#include <memory>

class A
{
  public:
    int mA = 0;
};

class B
{
  private:
    std::shared_ptr<A> mA;

  public:
    B(std::shared_ptr<A> a) : mA(a)
    {
    }
    int GetA() const
    {
        return mA->mA;
    }
};

TEST(DITest, Modify)
{
    auto injector = boost::di::make_injector(boost::di::bind<A>().to<A>().in(boost::di::singleton),
                                             boost::di::bind<B>().to<B>().in(boost::di::singleton));
    auto a = injector.create<std::shared_ptr<A>>();
    auto b = injector.create<std::shared_ptr<B>>();
    EXPECT_EQ(b->GetA(), 0);
    a->mA = 10;
    EXPECT_EQ(b->GetA(), 10);
}