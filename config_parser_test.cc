#include "gtest/gtest.h"
#include "config_parser.h"

class NginxStringConfigTest : public ::testing::Test {
protected:
    bool ParseString(const std::string &config_string) {
        std::stringstream config_stream(config_string);
        return parser_.Parse(&config_stream, &config_);
    }
    NginxConfigParser parser_;
    NginxConfig config_;
};

TEST(NginxConfigParserTest, SimpleConfig) {
    NginxConfigParser parser;
    NginxConfig out_config;
    bool success = parser.Parse("example_config", &out_config);
    EXPECT_TRUE(success);
}

TEST(NginxConfigParserTest, ComplexConfig) {
    NginxConfigParser parser;
    NginxConfig out_config;
    bool success = parser.Parse("complex_config", &out_config);
    EXPECT_TRUE(success);
}

TEST_F(NginxStringConfigTest, SimpleStatement) {
    std::string config_string("foo bar;");
    EXPECT_TRUE(ParseString(config_string));
    EXPECT_EQ(1, config_.statements_.size());
    EXPECT_EQ("foo", config_.statements_.at(0)->tokens_.at(0));
    EXPECT_EQ("bar", config_.statements_.at(0)->tokens_.at(1));
}

TEST_F(NginxStringConfigTest, ComplexStatement) {
    std::string config_string("foo bar 127.0.0.1 loo=dar;");
    EXPECT_TRUE(ParseString(config_string));
    EXPECT_EQ("foo", config_.statements_.at(0)->tokens_.at(0));
    EXPECT_EQ("bar", config_.statements_.at(0)->tokens_.at(1));
    EXPECT_EQ("127.0.0.1", config_.statements_.at(0)->tokens_.at(2));
    EXPECT_EQ("loo=dar", config_.statements_.at(0)->tokens_.at(3));
}

TEST_F(NginxStringConfigTest, StatementQuotes) {
    std::string config_string("foo bar \"\'quotes\'\";");
    EXPECT_TRUE(ParseString(config_string));
    EXPECT_EQ("foo", config_.statements_.at(0)->tokens_.at(0));
    EXPECT_EQ("bar", config_.statements_.at(0)->tokens_.at(1));
    EXPECT_EQ("\"\'quotes\'\"", config_.statements_.at(0)->tokens_.at(2));
}

TEST_F(NginxStringConfigTest, Comment) {
    std::string config_string("foo bar; # comment here");
    EXPECT_TRUE(ParseString(config_string));
    EXPECT_EQ(1, config_.statements_.size());
    EXPECT_EQ("foo", config_.statements_.at(0)->tokens_.at(0));
    EXPECT_EQ("bar", config_.statements_.at(0)->tokens_.at(1));
}

TEST_F(NginxStringConfigTest, ParseEmpty) {
    std::string config_string("");
    EXPECT_FALSE(ParseString(config_string));
}

TEST_F(NginxStringConfigTest, NestedBlock) {
    std::string config_string("foo { bar loo; }");
    EXPECT_TRUE(ParseString(config_string));
    EXPECT_EQ("foo", config_.statements_.at(0)->tokens_.at(0));
    EXPECT_EQ("bar", config_.statements_.at(0)->child_block_.get()
                        ->statements_.at(0)->tokens_.at(0));
    EXPECT_EQ("loo", config_.statements_.at(0)->child_block_.get()
                        ->statements_.at(0)->tokens_.at(1));
}

TEST_F(NginxStringConfigTest, NestedMultiple) {
    std::string config_string("foo { bar { loo dar; } }");
    EXPECT_TRUE(ParseString(config_string));
    EXPECT_EQ("foo", config_.statements_.at(0)->tokens_.at(0));
    EXPECT_EQ("bar", config_.statements_.at(0)->child_block_.get()
                        ->statements_.at(0)->tokens_.at(0));
    EXPECT_EQ("loo", config_.statements_.at(0)->child_block_.get()
                        ->statements_.at(0)->child_block_.get()
                        ->statements_.at(0)->tokens_.at(0));
    EXPECT_EQ("dar", config_.statements_.at(0)->child_block_.get()
                        ->statements_.at(0)->child_block_.get()
                        ->statements_.at(0)->tokens_.at(1));
}

TEST(NginxConfigTest, ToStringSimple) {
    NginxConfigStatement statement;
    statement.tokens_.push_back("foo");
    statement.tokens_.push_back("bar");
    EXPECT_EQ("foo bar;\n", statement.ToString(0));
}
