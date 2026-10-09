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

	static FieldInfo? sceneObjectTypeID;
	static FieldInfo? sceneObjectNativePointer;
	static Type? sceneObjectType;
	static MethodInfo? sceneObjectBegin = null;
	static MethodInfo? sceneObjectUpdate = null;
	static MethodInfo? sceneObjectOnDestroyed = null;

	static IntPtr typeIdIndex = 0;
	static IntPtr objectIdIndex = 1;

	readonly static Dictionary<IntPtr, ObjectTypeInfo> loadedTypes = [];
	readonly static Dictionary<IntPtr, object> loadedObjects = [];

	public static Delegate CreateDelegate(this MethodInfo methodInfo, object target)
	{
		Func<Type[], Type> getType;
		var isAction = methodInfo.ReturnType.Equals((typeof(void)));
		var types = methodInfo.GetParameters().Select(p => p.ParameterType);

		if (isAction)
		{
			getType = Expression.GetActionType;
		}
		else
		{
			getType = Expression.GetFuncType;
			types = types.Concat([methodInfo.ReturnType]);
		}

		if (methodInfo.IsStatic)
		{
			return Delegate.CreateDelegate(getType([.. types]), methodInfo);
		}

		return Delegate.CreateDelegate(getType([.. types]), target, methodInfo.Name);
	}

	public static void UpdateObjects()
	{
		foreach (var i in loadedObjects)
		{
			sceneObjectUpdate!.Invoke(i.Value, []);
		}
	}

	public static void LoadObjects(Assembly targetAssembly, Assembly engineAssembly)
	{
		sceneObjectType = engineAssembly.GetType("Engine.SceneObject");

		if (sceneObjectType == null)
			return;

		sceneObjectBegin = sceneObjectType.GetMethod("BeginInternal", BindingFlags.Public | BindingFlags.Instance)!;
		sceneObjectUpdate = sceneObjectType.GetMethod("Update")!;
		sceneObjectOnDestroyed = sceneObjectType.GetMethod("OnDestroyedInternal")!;

		sceneObjectNativePointer = sceneObjectType.GetField("NativePointer", BindingFlags.Public | BindingFlags.NonPublic | BindingFlags.Instance);
		sceneObjectTypeID = sceneObjectType.GetField("CSharpType", BindingFlags.Public | BindingFlags.NonPublic | BindingFlags.Instance);

		var registerObject = GetFunction<RegisterObject>("RegisterCSharpObject")!;

		foreach (var exported in targetAssembly.ExportedTypes)
		{
			if (!exported.IsSubclassOf(sceneObjectType))
				return;

			loadedTypes.Add(typeIdIndex, new ObjectTypeInfo
			{
				ObjectType = exported,
				Name = exported.ToString(),
			});

			registerObject.DynamicInvoke(exported.ToString(), typeIdIndex);
			typeIdIndex++;
		}
	}

	public delegate void ObjectFunction(IntPtr target);

	public static void RemoveObjectInstance(IntPtr objectID)
	{
		try
		{
			object destroyedObject = loadedObjects[objectID]!;

			sceneObjectOnDestroyed!.Invoke(destroyedObject, []);
			sceneObjectNativePointer!.SetValue(destroyedObject, IntPtr.Zero);
			loadedObjects.Remove(objectID);
		}
		catch (Exception e)
		{
			Console.WriteLine(e.ToString());
		}
	}

	public static IntPtr CreateObjectInstance(IntPtr type, IntPtr nativeObject)
	{
		try
		{
			ObjectTypeInfo loaded = loadedTypes[type];

			object? newObject = Activator.CreateInstance(loaded.ObjectType);

			if (newObject != null)
			{
				sceneObjectTypeID!.SetValue(newObject, type);
				sceneObjectNativePointer!.SetValue(newObject, nativeObject);
			}

			if (newObject == null)
				return IntPtr.Zero;

			sceneObjectBegin!.Invoke(newObject, []);

			loadedObjects.Add(objectIdIndex, newObject);
			return objectIdIndex++;
		}
		catch (Exception ex)
		{
			log!.Invoke(ex.ToString());
		}
		return 0;
	}
}
