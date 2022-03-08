#include "simulator.h"
#include "relaxivity_distribution.h"

namespace rw
{

Simulator::Simulator()
{
	this->_totalParticles = 0;
	this->_distribution = 0;
}

void Simulator::Set_Relaxivity_Distribution(rw::RelaxivityDistribution* distribution)
{
	this->_distribution = distribution;
	this->Configure_Relaxivity_Distribution(*this->_distribution);
}

const rw::RelaxivityDistribution& Simulator::RelaxivityDistribution() const
{
	return(*this->_distribution);
}



void Simulator::Set_NumberOfParticles(int particles)
{
	this->_totalParticles = particles;
	this->Set_Particle_Array();
}

Simulator::~Simulator()
{
	if (this->_distribution)
	{
		delete this->_distribution;
	}
}

void Simulator::ReleaseDistribution()
{
	this->_distribution = 0;
}

scalar Simulator::Magnetization(int id) const
{
	return(0);
}

int Simulator::Killed_Particles() const
{
	return(0);
}

int Simulator::Hits(int id) const
{
	return(0);
}

scalar Simulator::Delta(scalar xi) const
{
	return(this->_distribution->Relaxivity_Factor(this->_distribution->Evaluate(xi)));
}

scalar Simulator::Rho(scalar xi) const
{
	return(this->_distribution->Evaluate(xi));
}

void Simulator::Set_Gradient(const Field3D& field)
{
	this->_gradient = field;
}

const Field3D& Simulator::Gradient()
{
	return(this->_gradient);
}



}