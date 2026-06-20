#include "stdafx.h"
#pragma hdrstop

template XRCORE_API Fquaternion::SelfRef Fquaternion::set(float W, float X, float Y, float Z);
template XRCORE_API Fquaternion::SelfRef Fquaternion::set(Fquaternion::SelfCRef Q);
template XRCORE_API Fquaternion::SelfRef Fquaternion::set(const Fmatrix &m);
template XRCORE_API Fquaternion::SelfRef Fquaternion::rotationYawPitchRoll(float _x, float _y, float _z);
template XRCORE_API Fquaternion::SelfRef Fquaternion::slerp(Fquaternion::SelfCRef Q0, Fquaternion::SelfCRef Q1, float tm);
