#include <iostream>
#include"Vector3.h"

int main()
{
	Vector3<float> a;
	Vector3<float> b(1,0,1);
	Vector3<float> c(b);
	Vector3<float> d = b + c;

	a.print();
	b.print();
	c.print();
	d.print();

	d.Normalize().print();

	std::printf("%.2f\n", d.distance_to(b));

	std::printf("%.2f\n", d.dot_product(b));

	d.cross_product(b).print();

	std::printf("%.2f\n", d.angle_between(b));
}
