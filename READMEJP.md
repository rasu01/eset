# eset
([English Version Here](README.md))<br/>
esetと言うのは、英語のエンティティセットの略語であり、このプロジェクトの一番大事なのは簡単さと速度である。
大量のエンティティをなるべく早く繰り返すためには、各エンティティの種類ごとにアーキタイプがあり、その中にはアーキタイプの種類と一致するエンティティしか含まれなく、メモリにぎっしりと配置されます。
それぞれのエンティティの中にいろいろな要素を入れることができ、入っている要素の配置によってアーキタイプの種類が決められます。新しいエンティティを作ったり、もう存在するエンティティは要素を追加したり、削除したりすると、別のアーキタイプに自動的に移動されます。
要素は、どんなクラスやストラクトも有効とされます。
繰り返す直前に、指定した要素が含まれているエンティティしか繰り返しません。
各々のアーキタイプに存在するエンティティは同じ種類のため、アーキタイプの種類を確認することしか必要ありません。

# 繰り返し方
エンティティに含まれている希望する要素を指定し、普通のfor文のように繰り返します。このプログラムが要素のコンテンツを出力します。
指定した要素が含まれていないエンティティは無視されます。
```C++
//要素の宣言
class ClassComponent {
  float a;
  float b;
};

struct StructComponent {
  int c;
  int d;
};

//セットを作ります
eset::Set set;

/* ここではセットにエンティティとその要素も追加します */

//繰り返します！
for(auto [entity_id, class_comp, struct_comp] : set.iterator<ClassComponent, StructComponent>()) {
  std::cout << class_comp.a << ", " << class_comp.b << "; " << struct_comp.c << ", " << struct_comp.d << "\n";
}
```

# エンティティの変更の仕方
セットから新しいエンティティを取得し、そのエンティティの中に要素を追加したり、削除したりすることができ、エンティティ自体も削除することができます。
```C++
//要素の宣言
class Person {
  Person(std::string n, int a) {
    name = n;
    age = a;
  }

  public:
    std::string name;
    int age;
}; 

//セットを作ります
eset::Set set;

//エンティティを作って取得します
eset::Entity entity = set.create();

//エンティティに要素を追加します
set.insert<Person>(entity, Person("田中", 35));
set.insert<float>(entity, 50.0);

//エンティティに含まれている要素の参照を取得します
eset::Ref<Person> person = set.get<Person>(entity);
eset::Ref<float> floating_number = set.get<float>(entity);

//参照を使用する前、有効かどうかを確認します
if(person.valid() && floating_number.valid()) {
  std::cout << person.get()->name << "の年齢は、" << person.get()->age << "です。";
  std::cout << *(floating_number.get()) << "\n";
  //出力は: 田中の年齢は、35です。50.0
}

//エンティティを削除します
set.remove(entity);

/* ここから、エンティティが存在しないため、personとfloating_numberは有効ではありません */
```
