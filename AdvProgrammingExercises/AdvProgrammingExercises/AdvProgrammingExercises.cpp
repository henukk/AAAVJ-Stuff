#include <iostream>
#include"Vector3.h"
#include"String.h"

void exampleVect3() {
	std::cout << "Slide exercise Vector3:\n";
	Vector3<float> a;
	Vector3<float> b(1, 0, 1);
	Vector3<float> c(b);
	Vector3<float> d = b + c;

	a.print();
	b.print();
	c.print();
	d.print();

	d.Normalize().print();

	std::cout << d.distance_to(b) << "\n";

	std::cout << d.dot_product(b) << "\n";

	d.cross_product(b).print();

	std::cout << d.angle_between(b) << "\n";

	std::cout << "\n";
}

void exampleStrings() {
	std::cout << "Slide exercise String:\n";
	String a("hello");
	String b(a);
	String c = a + b;

	std::cout << a << ' ' << b << ' ' << c << '\n';
	a.print();
	b.print();
	c.print();

	if (c == "hellohello") {
		std::cout << "YES" << '\n';
	} else {
		std::cout << "NO" << '\n';
	}

	std::cout << c.length() << '\n';
	c.clear();
	c.print();

	std::cout << "\n";
}

int main()
{
	std::ios::sync_with_stdio(false);
	std::cin.tie(nullptr);

	exampleVect3();
	exampleStrings();
}
