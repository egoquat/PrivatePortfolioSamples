#pragma once

#include <PSSG.h>

namespace TRIPART
{
	class CTri
	{
	public:
		Vector3			v3FaceNormal;
		Vector3			v3PT[3];
		unsigned int	uiIV[3];

	public:
		// @ Set / Get
		void Set(	const CTri &triSrc);
		void Set(	const Vector3 *v3Ver_, const unsigned int *uiIV_);
		void Set(	const Vector3 &v01_,const Vector3 &v02_,const Vector3 &v03_,
					const unsigned int uiIV01, const unsigned int uiIV02, const unsigned int uiIV03 );

		void GetCenterTri(	Vector3 &v3Center );

	public:
		CTri();
		~CTri();
	};
}; // TRIPART

