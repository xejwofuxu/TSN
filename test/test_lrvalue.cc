#include <iostream>
#include <gtest/gtest.h>

void foo(int &a)
{
    std::cout << "int &: " << std::to_string(a) << std::endl;
    a = 3;
}

void foo(int &&a)
{
    std::cout << "int &&: " << std::to_string(a) << std::endl;
}

void foo(const int &a)
{
    std::cout << "const int &: " << std::to_string(a) << std::endl;
}

void Test_LRvalue()
{
    int a = 1;
    const int b = 2;
    foo(a);
    foo(std::move(a));
    foo(b);
}

// https://blog.csdn.net/Z_xOOOO/article/details/119515300
// 左值的英文简写为“lvalue”，右值的英文简写为“rvalue”。
// 很多人认为它们分别是"left value"、"right value" 的缩写，其实不然。
// lvalue 是“loactor value”的缩写，可意为存储在内存中、有明确存储地址（可寻址）的数据
// 而 rvalue 译为 "read value"，指的是那些可以提供数据值的数据（不一定可以寻址，例如存储于寄存器中的数据）。

TEST(TEST_LRVALUE, TEST_LRVALUE_READ) {
    Test_LRvalue();
}