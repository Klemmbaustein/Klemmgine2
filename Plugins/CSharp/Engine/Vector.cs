using System.Runtime.InteropServices;

namespace Engine;

[StructLayout(LayoutKind.Sequential)]
public struct Vector3
{
	public float X = 0;
	public float Y = 0;
	public float Z = 0;

	public Vector3()
	{

	}

	public Vector3(float XYZ)
	{
		X = XYZ;
		Y = XYZ;
		Z = XYZ;
	}

	public Vector3(float X, float Y, float Z)
	{
		this.X = X;
		this.Y = Y;
		this.Z = Z;
	}
}
