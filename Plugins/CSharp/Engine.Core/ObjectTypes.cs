using System.Linq.Expressions;
using System.Reflection;
using System.Runtime.InteropServices;
using static Engine.Core.Native;

namespace Engine.Core;

internal static class ObjectTypes
{
	public delegate void RegisterObject([MarshalAs(UnmanagedType.LPUTF8Str)] string Name, IntPtr Type);
	public delegate IntPtr CreateObjectInstanceDelegate(IntPtr Type, IntPtr Object);
	public delegate void RemoveObjectInstanceDelegate(IntPtr Type);

	struct ObjectTypeInfo
	{
		public Type ObjectType;
		public string Name;
	}

	static FieldInfo? SceneObjectTypeID;
	static FieldInfo? SceneObjectNativePointer;
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

	public static void UpdateObjects()
	{
		foreach (var i in LoadedObjects)
		{
			SceneObjectUpdate!.Invoke(i.Value, []);
		}
	}

	public static void LoadObjects(Assembly TargetAssembly, Assembly EngineAssembly)
	{
		SceneObjectType = EngineAssembly.GetType("Engine.SceneObject");

		if (SceneObjectType == null)
			return;

		SceneObjectBegin = SceneObjectType.GetMethod("BeginInternal", BindingFlags.Public | BindingFlags.Instance)!;
		SceneObjectUpdate = SceneObjectType.GetMethod("Update")!;
		SceneObjectOnDestroyed = SceneObjectType.GetMethod("OnDestroyedInternal")!;

		SceneObjectNativePointer = SceneObjectType.GetField("NativePointer", BindingFlags.Public | BindingFlags.NonPublic | BindingFlags.Instance);
		SceneObjectTypeID = SceneObjectType.GetField("CSharpType", BindingFlags.Public | BindingFlags.NonPublic | BindingFlags.Instance);

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

	public static void RemoveObjectInstance(IntPtr ObjectID)
	{
		try
		{
			object DestroyedObject = LoadedObjects[ObjectID]!;

			SceneObjectOnDestroyed!.Invoke(DestroyedObject, []);
			SceneObjectNativePointer!.SetValue(DestroyedObject, IntPtr.Zero);
			LoadedObjects.Remove(ObjectID);
		}
		catch (Exception e)
		{
			Console.WriteLine(e.ToString());
		}
	}

	public static IntPtr CreateObjectInstance(IntPtr Type, IntPtr NativeObject)
	{
		try
		{
			ObjectTypeInfo Loaded = LoadedTypes[Type];

			object? New = Activator.CreateInstance(Loaded.ObjectType);

			if (New != null)
			{
				SceneObjectTypeID!.SetValue(New, Type);
				SceneObjectNativePointer!.SetValue(New, NativeObject);
			}

			if (New == null)
				return IntPtr.Zero;

			SceneObjectBegin!.Invoke(New, []);

			LoadedObjects.Add(ObjectIdIndex, New);
			return ObjectIdIndex++;
		}
		catch (Exception ex)
		{
			Log!.Invoke(ex.ToString());
		}
		return 0;
	}
}
