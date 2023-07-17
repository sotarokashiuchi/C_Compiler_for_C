構文解析
- 抽象構文木を構築することを目標とする
- BNFで記述された生成規則を基に構文木を作成する

構文木 
具象構文木 :EBNFをそのまま表した構文木。2分木ではないため式の評価順序を表現できていない
抽象構文木 :構文木のうち括弧などの冗長な要素を木の中に残さずにコンパクトにした構文木。一般的に2分木で表されており、評価順序が一意に決まる
ex)1+2+3+4
具象構文木
     ---expr----
    / /  /|\  \ \
   / /  / | \  \ \
num + num + num + num
 |     |     |     |
 1     2     3     4
抽象構文木
      +
     / \
    +   4
   / \
  +   3
 / \
1   2

生成規則 :文法を再帰的に定義するルール
文脈自由文法 :EBNFで記述することができる生成規則のこと
BNF, EBNF

入力文字列
| トークナイズ(字句解析)
トークン文字列
| パース(構文解析)(BNF記法に基づき具象構造木を生成する過程を踏む場合もある)
抽象構文木
| 
スタックマシン用コード