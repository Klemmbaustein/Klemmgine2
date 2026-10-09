using System.Runtime.InteropServices;

namespace Engine;

[StructLayout(LayoutKind.Sequential)]
public struct Vector3
{
	public float X { get; set; } = 0;
	public float Y { get; set; } = 0;
	public float Z { get; set; } = 0;

	public Vector3()
	{

	}

	public Vector3(float xyz)
	{
		X = xyz;
		Y = xyz;
		Z = xyz;
	}

	public Vector3(float x, float y, float z)
	{
		X = x;
		Y = y;
		Z = z;
	}
}
