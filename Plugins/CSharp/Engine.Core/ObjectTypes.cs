using System.Linq.Expressions;
using System.Reflection;
using System.Runtime.InteropServices;
using static Engine.Core.Native;

namespace Engine.Core;

internal static class ObjectTypes
{
	struct ObjectTypeInfo
	{
		public Type ObjectType;
		public string Name;
		public MethodInfo Begin;
		public MethodInfo Update;
		public MethodInfo OnDestroyed;
	}

	static FieldInfo? SceneObjectTypeID;
	static Type? SceneObjectType;

	static IntPtr TypeIdIndex = 0;
	static IntPtr ObjectIdIndex = 1;

	readonly static Dictionary<IntPtr, ObjectTypeInfo> LoadedTypes = [];
	readonly static Dictionary<IntPtr, object> LoadedObjects = [];

	public static Delegate CreateDelegate(this MethodInfo methodInfo, object target)
	{
		Func<Type[], Type> GetType;
		var IsAction = methodInfo.ReturnType.Equals((typeof(void)));
		var Types = methodInfo.GetParameters().Select(p => p.ParameterType);

		if (IsAction)
		{
			GetType = Expression.GetActionType;
		}
		else
		{
			GetType = Expression.GetFuncType;
			Types = Types.Concat([methodInfo.ReturnType]);
		}

		if (methodInfo.IsStatic)
		{
			return Delegate.CreateDelegate(GetType([.. Types]), methodInfo);
		}

		return Delegate.CreateDelegate(GetType([.. Types]), target, methodInfo.Name);
	}

	//static IntPtr ToPointer(MethodInfo Method)
	//{
	//	foreach (var p in Method.GetParameters())
	//	{
	//		Console.WriteLine(p.Name);
	//	}
	//	return Marshal.GetFunctionPointerForDelegate(Method.CreateDelegate<ObjectFunction>());
	//}

	public delegate void RegisterObject([MarshalAs(UnmanagedType.LPUTF8Str)] string Name, IntPtr Type);

	public static void LoadObjects(Assembly TargetAssembly, Assembly EngineAssembly)
	{
		SceneObjectType = EngineAssembly.GetType("Engine.SceneObject");

		if (SceneObjectType == null)
			return;

		foreach (var i in SceneObjectType.GetFields())
		{
			Console.WriteLine(i.Name);
		}

		SceneObjectTypeID = SceneObjectType.GetField("CSharpType", BindingFlags.NonPublic | BindingFlags.Instance);

		var RegisterObject = GetFunction<RegisterObject>("RegisterCSharpObject")!;

		foreach (var Exported in TargetAssembly.ExportedTypes)
		{
			if (!Exported.IsSubclassOf(SceneObjectType))
				return;

			MethodInfo[] Methods = [
				Exported.GetMethod("Begin")!,
				Exported.GetMethod("Update")!,
				Exported.GetMethod("OnDestroyed")!
			];

			LoadedTypes.Add(TypeIdIndex, new ObjectTypeInfo
			{
				ObjectType = Exported,
				Name = Exported.ToString(),
				Begin = Methods[0],
				Update = Methods[1],
				OnDestroyed = Methods[2]
			});

			RegisterObject.DynamicInvoke(Exported.ToString(), TypeIdIndex);
			TypeIdIndex++;
		}
	}

	public delegate void ObjectFunction(IntPtr Target);

	public static void PrintObjectName(IntPtr Target)
	{
		Console.WriteLine(LoadedObjects[Target]);
	}


	public delegate IntPtr CreateObjectInstanceDelegate(IntPtr Type);
	[return: MarshalAs(UnmanagedType.U8)]
	public static IntPtr CreateObjectInstance(IntPtr Type)
	{
		ObjectTypeInfo Loaded = LoadedTypes[Type];

		object? New = Activator.CreateInstance(Loaded.ObjectType);

		if (New != null)
		{
			SceneObjectTypeID!.SetValue(New, Type);
		}

		Console.WriteLine($"Create: {New ?? "no object :("}");

		if (New == null)
			return IntPtr.Zero;

		LoadedObjects.Add(ObjectIdIndex, New);
		GCHandle.Alloc(New);
		return ObjectIdIndex++;
	}
}
