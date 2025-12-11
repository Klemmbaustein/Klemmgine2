using System.Diagnostics.CodeAnalysis;
using System.Runtime.InteropServices;
using System.Runtime.Loader;
using System.Text.RegularExpressions;
using Engine.Native;

namespace Engine.Aot;

internal partial class AotObjects
{
	public delegate void RegisterObject([MarshalAs(UnmanagedType.LPUTF8Str)] string Name, IntPtr Type);
	public delegate IntPtr CreateObjectInstanceDelegate(IntPtr Type);
	public delegate void RemoveObjectInstanceDelegate(IntPtr Type);

	struct ObjectTypeInfo
	{
		[DynamicallyAccessedMembers(DynamicallyAccessedMemberTypes.PublicParameterlessConstructor)]
		public Type ObjectType;
		public string Name;
	}

	static IntPtr TypeIdIndex = 0;
	static IntPtr ObjectIdIndex = 1;

	readonly static Dictionary<IntPtr, ObjectTypeInfo> LoadedTypes = [];
	readonly static Dictionary<IntPtr, SceneObject> LoadedObjects = [];

	public static Delegate? GetFunction<T>(string Name)
	{
		return Marshal.GetDelegateForFunctionPointer<T>(NativeFunctions.LoadedFunctions[Name]) as Delegate;
	}

	const string ObjectReflectionReason = "Uses reflection to dynamically get object types";

	[RequiresUnreferencedCode(ObjectReflectionReason)]
	static IEnumerable<Type> GetAllObjectTypes() => AssemblyLoadContext.Default.Assemblies
			.SelectMany((asm) => asm.GetTypes())
			.Where((type) => type.IsSubclassOf(typeof(SceneObject)));

	[RequiresUnreferencedCode(ObjectReflectionReason)]
	internal static void LoadObjects()
	{
		var SceneObjectTypes = GetAllObjectTypes();

		var RegisterObjectFunc = GetFunction<RegisterObject>("RegisterCSharpObject")!;

		foreach (Type ObjectType in SceneObjectTypes)
		{
			LoadedTypes.Add(TypeIdIndex, new ObjectTypeInfo
			{
				ObjectType = ObjectType,
				Name = ObjectType.ToString(),
			});

			RegisterObjectFunc.DynamicInvoke(ObjectType.ToString(), TypeIdIndex);
			TypeIdIndex++;
		}
	}


	public static void UpdateObjects()
	{
		foreach (var i in LoadedObjects)
		{
			i.Value.Update();
		}
	}

	[UnmanagedCallersOnly(EntryPoint = "Aot_RemoveObjectInstance")]
	internal static void RemoveObjectInstance(IntPtr ObjectID, IntPtr NativeObject)
	{
		try
		{
			SceneObject DestroyedObject = LoadedObjects[ObjectID]!;

			DestroyedObject.OnDestroyedInternal();
			DestroyedObject.NativePointer = 0;
			LoadedObjects.Remove(ObjectID);
		}
		catch (Exception e)
		{
			Console.WriteLine(e.ToString());
		}
	}

	[UnmanagedCallersOnly(EntryPoint = "Aot_CreateObjectInstance")]
	internal static IntPtr CreateObjectInstance(IntPtr Type, IntPtr NativeObject)
	{
		ObjectTypeInfo Loaded = LoadedTypes[Type];

		SceneObject? New = Activator.CreateInstance(Loaded.ObjectType) as SceneObject;

		if (New != null)
		{
			New.CSharpType = Type;
			New.NativePointer = NativeObject;
		}

		if (New == null)
			return IntPtr.Zero;

		New.BeginInternal();

		LoadedObjects.Add(ObjectIdIndex, New);
		return ObjectIdIndex++;
	}
}
