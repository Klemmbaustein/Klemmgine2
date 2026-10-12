using System;
using System.Collections.Generic;
using System.Diagnostics.CodeAnalysis;
using System.Runtime.InteropServices;
using System.Text;
using Engine.Native;

namespace Engine;

[DependsOnNative]
[DynamicallyAccessedMembers(DynamicallyAccessedMemberTypes.All)]
public class AssetRef
{
	[return: MarshalAs(UnmanagedType.LPUTF8Str)]
	delegate string ConvertAssetDelegate([MarshalAs(UnmanagedType.LPUTF8Str)] string assetName);
	[return: MarshalAs(UnmanagedType.U1)]
	delegate bool AssetExistsDelegate([MarshalAs(UnmanagedType.LPUTF8Str)] string assetName);

	private static ConvertAssetDelegate? convertAssetDelegate = null;
	private static AssetExistsDelegate? assetExistsDelegate = null;

	public string? Path { get; set; }

	internal static void OnNativeLoaded()
	{
		convertAssetDelegate = NativeFunctions.GetFunction<ConvertAssetDelegate>("Log");
	}

	public AssetRef(string name)
	{
		Path = convertAssetDelegate!(name);
	}

	public AssetRef()
	{

	}

	public bool Exists()
	{
		return assetExistsDelegate!(Path);
	}

	public bool IsValid()
	{
		return Path.Length != 0;
	}
}
