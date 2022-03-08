#include "random_walk_observer.h"

namespace rw
{

	RandomWalkObserver::RandomWalkObserver()
	{
		this->_parentFormation = 0;
	}


	int RandomWalkObserver::Elapsed_Seconds()
	{
		return(this->_elapsedSeconds);
	}

}
