/*
 * argparse-cpp Hello World Example
 * 
 * この例では、argparse-cppライブラリの基本的な使用方法を示します。
 * - 位置引数1つ（名前）
 * - オプション引数2-3個（--verbose, --count, --format）
 * - ヘルプメッセージの自動生成
 */

#include <iostream>
#include "../../include/argparse/argparse.hpp"

int main(int argc, char* argv[]) {
    try {
        // ArgumentParserを作成（プログラム名と説明を指定）
        argparse::ArgumentParser parser(
            "hello_world",                              // プログラム名
            "argparse-cpp demonstration program"       // 説明
        );

        // 位置引数を追加：名前（必須）
        parser.add_argument("name")
            .help("Your name to say hello to");

        // オプション引数を追加：--verbose/-v（フラグ）
        parser.add_argument("-v", "--verbose")
            .action("store_true")
            .help("Enable verbose output");

        // オプション引数を追加：--count/-c（整数）
        parser.add_argument("-c", "--count")
            .type("int")
            .default_value(1)
            .help("Number of times to say hello (default: 1)");

        // オプション引数を追加：--format/-f（選択肢）
        std::vector<std::string> format_choices = {"simple", "fancy", "caps"};
        parser.add_argument("-f", "--format")
            .choices(format_choices)
            .default_value(std::string("simple"))
            .help("Output format: simple, fancy, or caps (default: simple)");

        // コマンドライン引数を解析
        auto args = parser.parse_args(argc, argv);

        // 解析結果を取得
        std::string name = args.get<std::string>("name");
        bool verbose = args.get<bool>("verbose", false);  
        int count = args.get<int>("count", 1);            
        std::string format = args.get<std::string>("format", "simple");

        // Verboseモードの場合、設定を表示
        if (verbose) {
            std::cout << "Settings:" << std::endl;
            std::cout << "  Name: " << name << std::endl;
            std::cout << "  Count: " << count << std::endl;
            std::cout << "  Format: " << format << std::endl;
            std::cout << std::endl;
        }

        // 指定された回数だけ、指定されたフォーマットで挨拶
        for (int i = 0; i < count; ++i) {
            std::string greeting;
            
            if (format == "simple") {
                greeting = "Hello, " + name + "!";
            } else if (format == "fancy") {
                greeting = "🎉 Hello there, " + name + "! 🎉";
            } else if (format == "caps") {
                greeting = "HELLO, " + name + "!";
                // 大文字に変換
                std::transform(greeting.begin(), greeting.end(), greeting.begin(), ::toupper);
            }
            
            std::cout << greeting << std::endl;
        }

        return 0;

    } catch (const argparse::help_requested& e) {
        // ヘルプが要求された場合（--help/-h）
        std::cout << e.what() << std::endl;
        return 0;
    } catch (const std::exception& e) {
        // エラーが発生した場合
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}

/*
 * 使用例：
 * 
 * $ ./hello_world Alice
 * Hello, Alice!
 * 
 * $ ./hello_world Bob --verbose --count 3 --format fancy
 * Settings:
 *   Name: Bob
 *   Count: 3
 *   Format: fancy
 * 
 * 🎉 Hello there, Bob! 🎉
 * 🎉 Hello there, Bob! 🎉
 * 🎉 Hello there, Bob! 🎉
 * 
 * $ ./hello_world Charlie -c 2 -f caps
 * HELLO, CHARLIE!
 * HELLO, CHARLIE!
 * 
 * $ ./hello_world --help
 * [ヘルプメッセージが表示される]
 */