using System.Diagnostics.CodeAnalysis;
using System.Runtime.InteropServices;
using System.Runtime.Loader;
using System.Text.RegularExpressions;
using Engine.Native;

namespace Engine.Aot;

internal partial class AotObjects
{
	public delegate void RegisterObject([MarshalAs(UnmanagedType.LPUTF8Str)] string name, IntPtr type);
	public delegate IntPtr CreateObjectInstanceDelegate(IntPtr type);
	public delegate void RemoveObjectInstanceDelegate(IntPtr type);

	struct ObjectTypeInfo
	{
		[DynamicallyAccessedMembers(DynamicallyAccessedMemberTypes.PublicParameterlessConstructor)]
		public Type objectType;
		public string name;
	}

	static IntPtr typeIdIndex = 0;
	static IntPtr objectIdIndex = 1;

	readonly static Dictionary<IntPtr, ObjectTypeInfo> loadedTypes = [];
	readonly static Dictionary<IntPtr, SceneObject> loadedObjects = [];

	public static Delegate? GetFunction<T>(string Name)
	{
		return Marshal.GetDelegateForFunctionPointer<T>(NativeFunctions.loadedFunctions[Name]) as Delegate;
	}

	const string ObjectReflectionReason = "Uses reflection to dynamically get object types";

	[RequiresUnreferencedCode(ObjectReflectionReason)]
	static IEnumerable<Type> GetAllObjectTypes() => AssemblyLoadContext.Default.Assemblies
			.SelectMany((asm) => asm.GetTypes())
			.Where((type) => type.IsSubclassOf(typeof(SceneObject)));

	[RequiresUnreferencedCode(ObjectReflectionReason)]
	internal static void LoadObjects()
	{
		var sceneObjectTypes = GetAllObjectTypes();

		var registerObjectFunc = GetFunction<RegisterObject>("RegisterCSharpObject")!;

		foreach (Type objectType in sceneObjectTypes)
		{
			loadedTypes.Add(typeIdIndex, new ObjectTypeInfo
			{
				objectType = objectType,
				name = objectType.ToString(),
			});

			registerObjectFunc.DynamicInvoke(objectType.ToString(), typeIdIndex);
			typeIdIndex++;
		}
	}


	public static void UpdateObjects()
	{
		foreach (var i in loadedObjects)
		{
			i.Value.Update();
		}
	}

	[UnmanagedCallersOnly(EntryPoint = "Aot_RemoveObjectInstance")]
	internal static void RemoveObjectInstance(IntPtr objectID, IntPtr _)
	{
		try
		{
			SceneObject destroyedObject = loadedObjects[objectID]!;

			destroyedObject.OnDestroyedInternal();
			destroyedObject.nativePointer = 0;
			loadedObjects.Remove(objectID);
		}
		catch (Exception e)
		{
			Console.WriteLine(e.ToString());
		}
	}

	[UnmanagedCallersOnly(EntryPoint = "Aot_CreateObjectInstance")]
	internal static IntPtr CreateObjectInstance(IntPtr type, IntPtr nativeObject)
	{
		ObjectTypeInfo loaded = loadedTypes[type];

		SceneObject? newObject = Activator.CreateInstance(loaded.objectType) as SceneObject;

		if (newObject != null)
		{
			newObject.CSharpType = type;
			newObject.nativePointer = nativeObject;
		}

		if (newObject == null)
			return IntPtr.Zero;

		newObject.BeginInternal();

		loadedObjects.Add(objectIdIndex, newObject);
		return objectIdIndex++;
	}
}
