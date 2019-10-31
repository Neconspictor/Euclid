#pragma once

namespace nex
{
	struct PCFFilter
	{
		unsigned sampleCountX;
		unsigned sampleCountY;
		bool useLerpFiltering;

		bool operator==(const PCFFilter& o) {
			return sampleCountX == o.sampleCountX
				&& (sampleCountY == o.sampleCountY)
				&& (useLerpFiltering == o.useLerpFiltering);
		}
	};
}