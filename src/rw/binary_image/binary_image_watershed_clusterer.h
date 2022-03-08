#ifndef WATERSHED_CLUSTERER_H
#define WATERSHED_CLUSTERER_H

#include "binary_image_executor.h"
#include "binary_image_group_mask.h"
#include "math_la/mdefs.h"

namespace rw
{
	class BinaryImageWaterShedClusterer : public BinaryImageExecutor
	{
	private:
		int _step;
	protected:
		void Init_Clusters(vec(int)& points, BinaryImage::ProgressAdapter* pgdlg);
		vec(rw::Pos3i) Neigborhood(int rad);
		void Cluster(const vec(int)& input, vec(int)& output, int rad, BinaryImage::ProgressAdapter* pgdlg = 0);
		void Median();
		void Update_Clusters();
	public:
		BinaryImageWaterShedClusterer(BinaryImageExecutor& executor);
		void Execute(BinaryImage::ProgressAdapter* pgdlg = 0);
	};
}



#endif

