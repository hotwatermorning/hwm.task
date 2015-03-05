hwm.task
========

### これは何

軽量なスレッドプールの実装

### 特徴

 * ヘッダーオンリー
 * C++標準スレッドを使用
 * 起動するスレッド数を指定できる
 * 実行するタスクの戻り値を`std::future`で取得できる。

### サンプル

```cpp
int main()
{
    //! タスクキュー
    //! キューに積まれた関数／関数オブジェクトを別スレッドで随時取り出して実行する。
    //! 実行するスレッドの数をコンストラクタで指定する。
    hwm::task_queue tq(1);

    std::future<int> f =
        tq.enqueue(
            //! タスクキュー内のスレッドで起動する関数
            [](int x1, int x2) -> int {
                hwm::mcout << (x1 + x2) << std::endl;
                return x1 + x2;
            },
            //! 関数に渡す引数
            10, 20
        );

    //! enqueue()関数の引数に渡したラムダ式が非同期で実行される

    //! ラムダ式の実行完了を待って、結果を表示
    int const result = f.get();

    hwm::mcout << "calculated value : " << result << std::endl;
}
```

その他実行例は `./libs/examples/task/*.cpp` にあるサンプルコードを参照。

サンプルコードのビルドには [scons](http://www.scons.org/)が必要。  
`./libs/examples`で`scons`を実行すると、各サンプルコードをビルドできる。（`./libs/examples/bin`以下に実行ファイルが生成される）

### 注意

 * `hwm::task_queue`はコンストラクタでスレッドを生成し、デストラクタでスレッドを終了する。
 * そのため、グローバル変数として`hwm::task_queue`のオブジェクトを定義したりすると、`main()`関数の実行前にスレッドが作成されたり、`main()`関数の実行後にスレッドが終了したりして、予期せぬエラーを引き起こすことがあるので注意する。

 * Visual Studioではutf-8エンコーディングのテキストファイルを正しく扱えず、Windowsに設定されたコードページの設定でマルチバイト文字を解釈しようとする。そのため、ソースファイルに漢字やひらがななどのマルチバイト文字が含まれていると、ソースの文字が誤って解釈されて正しくコンパイルできないことがある。
 * 特にhwm.taskのソースでは、コメントの終わりで改行が正しく解釈されずコンパイルエラーが発生したため、適宜コメントの終わりに空白文字を追加するという回避策を取っている。

### LICENSE

Copyright hotwatermorning 2013 - 2015.  
Distributed under the Boost Software License, Version 1.0.  
(See accompanying file LICENSE_1_0.txt or copy at  
http://www.boost.org/LICENSE_1_0.txt)  

