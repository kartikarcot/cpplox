/**
 * @file test_func.cpp
 * Tests for creating and evaluating functions
 */

#include "eval.h"
#include "lox.h"
#include "loxfun.h"
#include "resolver.h"
#include "gtest/gtest.h"

TEST(FunctionTest, ParseSimpleFunction) {
  std::string test_code = R"(
                            fun sum(a, b) {
                                return a + b;
                            }
                            var a = 9.0;
                            )";
  Scanner scanner;
  scanner.init(test_code);
  scanner.scan();
  Parser parser;
  parser.init(scanner.get_tokens());
  std::vector<std::shared_ptr<Stmt>> stmts = parser.parse_stmts();
  // assert that we have two statements
  // and that the first one is a function
  ASSERT_EQ(stmts.size(), 2);
  // check that statment is castable to a Function pointer
  ASSERT_TRUE(dynamic_cast<Function *>(stmts[0].get()));
  // assert that the num of args is 2
  // and that the body is a block
  // with one statement
  ASSERT_EQ(((Function *)stmts[0].get())->params.size(), 2);
  ASSERT_EQ(((Function *)stmts[0].get())->body.size(), 1);
}

// A test to parse and check if a function enters the environment
TEST(FunctionTest, EvalFunctionDeclaration) {
  std::string test_code = R"(
                            fun sum(a, b) {
                                return a + b;
                            }
                            var a = 9.0;
                            a = sum(1.0, 2.0);
                            )";
  Scanner scanner;
  scanner.init(test_code);
  scanner.scan();
  Parser parser;
  parser.init(scanner.get_tokens());
  std::vector<std::shared_ptr<Stmt>> stmts = parser.parse_stmts();
  Evaluator eval;
  Resolver resolver(&eval);
  bool ret = resolver.resolve(stmts);
  ASSERT_TRUE(ret);
  eval.eval(std::move(stmts));
  ASSERT_TRUE(eval.env->get("sum") != nullptr);
  ASSERT_TRUE(eval.env->get("a") != nullptr);
  // assert that the object val pointer is a LoxFunction ptr
  ASSERT_TRUE(eval.env->get("sum")->val != nullptr);
  ASSERT_TRUE(eval.env->get("sum")->type == ObjectType::FUNCTION);
  auto lf_ptr = (LoxFunction *)(eval.env->get("sum")->val);
  ASSERT_TRUE(lf_ptr->arity() == 2);
  // Assert the value of a
  ASSERT_TRUE(eval.env->get("a")->type == ObjectType::FLOAT);
  ASSERT_TRUE(eval.env->get("a")->val != nullptr);
  ASSERT_TRUE(*(float *)(eval.env->get("a")->val) == 3.0);
}

// Test a fibonacci function
TEST(FunctionTest, FibonacciFunction) {
  std::string test_code = R"(
                            fun fib(n) {
                                if (n <= 1) {
                                    return 1;
                                }
                                return fib(n - 1) + fib(n - 2);
                            }
                            var a = fib(6);
                            )";
  Scanner scanner;
  scanner.init(test_code);
  scanner.scan();
  Parser parser;
  parser.init(scanner.get_tokens());
  std::vector<std::shared_ptr<Stmt>> stmts = parser.parse_stmts();
  Evaluator eval;
  Resolver resolver(&eval);
  bool ret = resolver.resolve(stmts);
  ASSERT_TRUE(ret);

  eval.eval(std::move(stmts));
  ASSERT_TRUE(eval.env->get("fib") != nullptr);
  ASSERT_TRUE(eval.env->get("a") != nullptr);
  // assert that the object val pointer is a LoxFunction ptr
  ASSERT_TRUE(eval.env->get("fib")->val != nullptr);
  ASSERT_TRUE(eval.env->get("fib")->type == ObjectType::FUNCTION);
  auto lf_ptr = (LoxFunction *)(eval.env->get("fib")->val);
  ASSERT_TRUE(lf_ptr->arity() == 1);
  // Assert the value of a
  ASSERT_TRUE(eval.env->get("a")->type == ObjectType::FLOAT);
  ASSERT_TRUE(eval.env->get("a")->val != nullptr);
  ASSERT_TRUE(*(float *)(eval.env->get("a")->val) == 13.0);
}

// A test to check if function closure works
TEST(FunctionTest, ClosureFunctionTest) {
  std::string test_code = R"(
                                fun makeCounter() {
                                    var i = 0;
                                    fun count() {
                                        i = i + 1;
                                        return i;
                                    }
                                    return count;
                                }
                                var counter = makeCounter();
                                var a = counter(); // "1".
                                var b = counter(); // "2".
                                )";
  Scanner scanner;
  scanner.init(test_code);
  scanner.scan();
  Parser parser;
  parser.init(scanner.get_tokens());
  std::vector<std::shared_ptr<Stmt>> stmts = parser.parse_stmts();
  Evaluator eval;
  Resolver resolver(&eval);
  bool ret = resolver.resolve(stmts);
  ASSERT_TRUE(ret);

  eval.eval(std::move(stmts));
  ASSERT_TRUE(eval.env->get("makeCounter") != nullptr);
  ASSERT_TRUE(eval.env->get("counter") != nullptr);
  ASSERT_TRUE(eval.env->get("a") != nullptr);
  ASSERT_TRUE(eval.env->get("b") != nullptr);
  // assert that value of a is 1 and value of b is 2
  ASSERT_TRUE(eval.env->get("a")->type == ObjectType::FLOAT);
  ASSERT_TRUE(eval.env->get("a")->val != nullptr);
  ASSERT_TRUE(*(float *)(eval.env->get("a")->val) == 1.0);
  ASSERT_TRUE(eval.env->get("b")->type == ObjectType::FLOAT);
  ASSERT_TRUE(eval.env->get("b")->val != nullptr);
  ASSERT_TRUE(*(float *)(eval.env->get("b")->val) == 2.0);
}

// A test of frozen closure
TEST(FunctionTest, FrozenClosure) {
  std::string test_code = R"(
                                var a = "global";
                                var ret1 = "undefined";
                                var ret2 = "undefined";
                                {
                                    fun showA() {
                                      return a;
                                    }

                                    ret1 = showA();
                                    var a = "block";
                                    ret2 = showA();
                                }
                                )";
  Scanner scanner;
  scanner.init(test_code);
  scanner.scan();
  Parser parser;
  parser.init(scanner.get_tokens());
  std::vector<std::shared_ptr<Stmt>> stmts = parser.parse_stmts();
  Evaluator eval;
  Resolver resolver(&eval);
  resolver.resolve(stmts);
  eval.eval(std::move(stmts));
  // assert that ret1 and ret2 are equal to global
  ASSERT_TRUE(eval.env->get("ret1") != nullptr);
  ASSERT_TRUE(eval.env->get("ret2") != nullptr);
  ASSERT_TRUE(eval.env->get("ret1")->type == ObjectType::STR);
  ASSERT_TRUE(eval.env->get("ret2")->type == ObjectType::STR);
  ASSERT_TRUE(eval.env->get("ret1")->val != nullptr);
  ASSERT_TRUE(eval.env->get("ret2")->val != nullptr);
  ASSERT_TRUE(std::string((char *)(eval.env->get("ret1")->val)) == "global");
  ASSERT_TRUE(std::string((char *)(eval.env->get("ret2")->val)) == "global");
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
