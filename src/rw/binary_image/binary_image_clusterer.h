#ifndef BINARY_IMAGE_CLUSTERER_H
#define BINARY_IMAGE_CLUSTERER_H

#include "binary_image_executor.h"
#include "binary_image_group_mask.h"
#include "math_la/mdefs.h"
#include "box3d.h"
#include <map>

namespace rw
{
	using std::map;

	class BinaryImageClusterer : public BinaryImageExecutor, public BinaryImageGroupMask
	{
	private:
		int _step;
	protected:
		void Set_Rad_Min_To_Max();
		void Expand_Diameter(BinaryImage::ProgressAdapter* pgdlg);
		void Extend_Diameter(int diam, BinaryImage::ProgressAdapter* pgdlg);
		void Group_Clusters(int diam, BinaryImage::ProgressAdapter* pgdlg);

		/**
		* When clusters are grouped, a fusion map is constructed to combine overlapping clusters.
		* This method applies the map and classify the voxels, and this process is irreversible.
		* @param fusion Fusion map constructed
		* @param rad Radius of all overlapping spheres classified
		*/
		void Apply_Fusion_Map(const map<int, int>& fusion, int diam);
		void Apply_Fusion_Map(const map<int, int>& fusion);


		void Set_Positive_Distances();
		uint Number_Of_Steps() const;
		void Recluster(int diam, BinaryImage::ProgressAdapter* pgdlg);
		virtual void Extend_Box(rw::Box3D& box, const rw::Pos3i& center, int diam) const;
		virtual void Set_Box(rw::Box3D& box, const Pos3i& center, int diam) const;
		virtual int Square_Cluster_Distance(int diam) const;
	public:
		BinaryImageClusterer(BinaryImageExecutor& executor);
		void Execute(BinaryImage::ProgressAdapter* pgdlg = 0);
	};

	inline uint BinaryImageClusterer::Number_Of_Steps() const
	{
		return(this->Max_Radius() * 2+2);
	}
}

#endif

