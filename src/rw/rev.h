#ifndef REV_H
#define REV_H

#include <vector>
#include <random>
#include "plug.h"

namespace rw
{

using std::vector;

class Rev
{
public:
	class RevEvent
	{
	public:
		virtual void On_Walk_Event(int k, scalar percentage) = 0;
	};

private:
	std::ranlux48 _gen;
	Rev::RevEvent* _event;
	const Plug* _base;
	const rw::BinaryImage* _imgPtr;
	int _currentSectionSize;
	vector <Pos3i> _origins;
	vector <scalar> _porosities;

	vector <math_la::math_lac::full::Vector> _laplaces;
	vector <math_la::math_lac::full::Vector> _timeT2;

	uint _setSize;
	int _secX;
	int _secY;
	int _secZ;
	int _sizeUpperBound;

	scalar _porosityTestThreshold;
	int _subStep;
	int _nwStep;
	
	scalar _nwcorr;
	scalar _reg;

	void Porosity_Distribution(scalar& mean, scalar& std_dev, scalar& min, scalar& max);
	scalar Section_Porosity(const Pos3i& pp) const;
	void Build_Subsets();
public:
	Rev();
	bool Porosity_Test(scalar& mean, scalar& std_dev, scalar& min, scalar& max);
	void Set_Section_Size_Step(int size);
	void Set_Section_Size(int size);
	uint Section_Size() const;
	void Set_Sample_Size(uint size);
	uint Sample_Size() const;
	
	void Walk_Inside_Sections(int nw, scalar delta, scalar t2min, scalar t2max, uint res, scalar reg, scalar dt);
	void Section(int id, rw::Pos3i& origin, scalar& porosity) const;
	void Set_Image(const rw::BinaryImage& img);
	void Set_Porosity_Test_Threshold(scalar threshold);
	uint Section_Number_Of_Walkers() const;
	void Set_Event(rw::Rev::RevEvent* event);
	void Get_T2_Distribution(uint idx, math_la::math_lac::full::Vector& laplace, math_la::math_lac::full::Vector& T2v);
	bool Porosity_Test_Executed() const;
	bool Random_Walk_Executed() const;
	static scalar Unresolved_Porosity(scalar mean, scalar std_dev, scalar lab_pore, scalar confidence);
};

}


#endif