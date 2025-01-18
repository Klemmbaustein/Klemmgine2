using System.Linq.Expressions;
using System.Reflection;
using System.Runtime.InteropServices;
using static Engine.Core.Native;

namespace Engine.Core;

internal static class ObjectTypes
{
	public delegate void RegisterObject([MarshalAs(UnmanagedType.LPUTF8Str)] string Name, IntPtr Type);
	public delegate IntPtr CreateObjectInstanceDelegate(IntPtr Type);
	public delegate void RemoveObjectInstanceDelegate(IntPtr Type);

	struct ObjectTypeInfo
	{
		public Type ObjectType;
		public string Name;
	}

	static FieldInfo? SceneObjectTypeID;
	static Type? SceneObjectType;
	static MethodInfo? SceneObjectBegin = null;
	static MethodInfo? SceneObjectUpdate = null;
	static MethodInfo? SceneObjectOnDestroyed = null;

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

	public static void LoadObjects(Assembly TargetAssembly, Assembly EngineAssembly)
	{
		SceneObjectType = EngineAssembly.GetType("Engine.SceneObject");

		if (SceneObjectType == null)
			return;

		SceneObjectBegin = SceneObjectType.GetMethod("Begin")!;
		SceneObjectUpdate = SceneObjectType.GetMethod("Update")!;
		SceneObjectOnDestroyed = SceneObjectType.GetMethod("OnDestroyed")!;

		SceneObjectTypeID = SceneObjectType.GetField("CSharpType", BindingFlags.NonPublic | BindingFlags.Instance);

		var RegisterObject = GetFunction<RegisterObject>("RegisterCSharpObject")!;

		foreach (var Exported in TargetAssembly.ExportedTypes)
		{
			if (!Exported.IsSubclassOf(SceneObjectType))
				return;

			LoadedTypes.Add(TypeIdIndex, new ObjectTypeInfo
			{
				ObjectType = Exported,
				Name = Exported.ToString(),
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

	public static void RemoveObjectInstance(IntPtr ObjectID)
	{
		try
		{
			object DestroyedObject = LoadedObjects[ObjectID]!;

			SceneObjectOnDestroyed!.Invoke(DestroyedObject, []);
			LoadedObjects.Remove(ObjectID);
		}
		catch (Exception e)
		{
			Console.WriteLine(e.ToString());
		}
	}

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

		SceneObjectBegin!.Invoke(New, []);

		LoadedObjects.Add(ObjectIdIndex, New);
		return ObjectIdIndex++;
	}
}
