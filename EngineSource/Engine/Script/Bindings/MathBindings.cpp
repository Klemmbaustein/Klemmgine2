#include "MathBindings.h"
#include <ds/parser/types/stringType.hpp>
#include <Core/Vector.h>
#include <Core/Transform.h>
#include <Core/BoundingBox.h>
#include <ds/language.hpp>

using namespace ds;
using namespace engine;

static void Rotation3_forward(InterpretContext* context)
{
	context->pushValue(Vector3::Forward(context->popValue<Rotation3>().EulerVector()));
}

static void Rotation3_right(InterpretContext* context)
{
	context->pushValue(Vector3::Right(context->popValue<Rotation3>().EulerVector()));
}

static void Rotation3_up(InterpretContext* context)
{
	context->pushValue(Vector3::Up(context->popValue<Rotation3>().EulerVector()));
}

static void Vector3_add(InterpretContext* context)
{
	context->pushValue(context->popValue<Vector3>() + context->popValue<Vector3>());
}

static void Vector3_multiply(InterpretContext* context)
{
	context->pushValue(context->popValue<Vector3>() * context->popValue<Vector3>());
}

static void Vector3_subtract(InterpretContext* context)
{
	Vector3 b = context->popValue<Vector3>();
	Vector3 a = context->popValue<Vector3>();
	context->pushValue(a - b);
}

static void Vector3_divide(InterpretContext* context)
{
	Vector3 b = context->popValue<Vector3>();
	Vector3 a = context->popValue<Vector3>();
	context->pushValue(a / b);
}

static void Vector3_unaryMinus(InterpretContext* context)
{
	context->pushValue(-context->popValue<Vector3>());
}

static void Vector3_length(InterpretContext* context)
{
	context->pushValue(context->popValue<Vector3>().Length());
}

static void Vector3_dot(InterpretContext* context)
{
	Vector3 b = context->popValue<Vector3>();
	Vector3 a = context->popValue<Vector3>();

	context->pushValue(Vector3::Dot(a, b));
}

static void Vector3_cross(InterpretContext* context)
{
	Vector3 b = context->popValue<Vector3>();
	Vector3 a = context->popValue<Vector3>();

	context->pushValue(Vector3::Cross(a, b));
}

static void Vector3_normalized(InterpretContext* context)
{
	context->pushValue(context->popValue<Vector3>().Normalize());
}

static void Vector3_projectToPlane(InterpretContext* context)
{
	Vector3 ThisVec = context->popValue<Vector3>();
	Vector3 PlaneNormal = context->popValue<Vector3>();
	Vector3 PlaneOrigin = context->popValue<Vector3>();

	context->pushValue(ThisVec.ProjectToPlane(PlaneOrigin, PlaneNormal));
}

static void Vector3_toString(InterpretContext* context)
{
	Vector3 ThisVec = context->popValue<Vector3>();

	auto str = ThisVec.ToString();

	context->pushRuntimeString(RuntimeStr(str.data(), str.size()));
}

static void Transform_decompose(InterpretContext* context)
{
	script::DecomposeResult Result{};
	context->popValue<Transform>().Decompose(Result.Position, Result.Rotation, Result.Scale);

	context->pushValue(Result);
}

static void Transform_translate(InterpretContext* context)
{
	auto Value = context->popValue<Transform>();
	Value.Translate(context->popValue<Vector3>());
	context->pushValue(Value);
}

static void Transform_rotate(InterpretContext* context)
{
	auto Value = context->popValue<Transform>();
	Value.Rotate(context->popValue<Rotation3>());
	context->pushValue(Value);
}

static void Transform_scale(InterpretContext* context)
{
	auto Value = context->popValue<Transform>();

	Value.Scale(context->popValue<Vector3>());

	context->pushValue(Value);
}

static void Transform_rotateAround(InterpretContext* context)
{
	auto Value = context->popValue<Transform>();

	auto Amount = context->popValue<Float>();
	auto Axis = context->popValue<Vector3>();

	Value.RotateAround(Axis, Amount);

	context->pushValue(Value);
}

static void Transform_identity(InterpretContext* context)
{
	context->pushValue(Transform());
}

static void Transform_new(InterpretContext* context)
{
	auto Scale = context->popValue<Vector3>();
	auto Rotation = context->popValue<Rotation3>();
	auto Position = context->popValue<Vector3>();

	context->pushValue(Transform(Position, Rotation, Scale));
}

script::MathBindings engine::script::AddMathModule(ds::NativeModule& To, LanguageContext* ToContext)
{
	script::MathBindings Math;

	auto FloatInst = ToContext->registry->getEntry<FloatType>();
	auto StrInst = ToContext->registry->getEntry<StringType>();

	Math.Vec3 = DS_CREATE_STRUCT(Vector3);
	Math.Bounds = DS_CREATE_STRUCT(BoundingBox);
	Math.Transform = DS_CREATE_STRUCT(Transform);
	Math.Rot = DS_CREATE_STRUCT(Rotation3);
	Math.Vec2 = DS_CREATE_STRUCT(Vector2);

	auto DecomposeResultType = DS_CREATE_STRUCT(DecomposeResult);

	DS_STRUCT_MEMBER_NAME(Math.Vec3, Vector3, X, x, FloatInst);
	DS_STRUCT_MEMBER_NAME(Math.Vec3, Vector3, Y, y, FloatInst);
	DS_STRUCT_MEMBER_NAME(Math.Vec3, Vector3, Z, z, FloatInst);

	DS_STRUCT_MEMBER_NAME(Math.Bounds, BoundingBox, Position, position, Math.Vec3);
	DS_STRUCT_MEMBER_NAME(Math.Bounds, BoundingBox, Extents, extents, Math.Vec3);

	DS_STRUCT_MEMBER_NAME(DecomposeResultType, DecomposeResult, Position, position, Math.Vec3);
	DS_STRUCT_MEMBER_NAME(DecomposeResultType, DecomposeResult, Rotation, rotation, Math.Rot);
	DS_STRUCT_MEMBER_NAME(DecomposeResultType, DecomposeResult, Scale, scale, Math.Vec3);

	DS_STRUCT_MEMBER_NAME(Math.Rot, Rotation3, P, p, FloatInst);
	DS_STRUCT_MEMBER_NAME(Math.Rot, Rotation3, Y, y, FloatInst);
	DS_STRUCT_MEMBER_NAME(Math.Rot, Rotation3, R, r, FloatInst);

	DS_STRUCT_MEMBER_NAME(Math.Vec2, Vector2, X, x, FloatInst);
	DS_STRUCT_MEMBER_NAME(Math.Vec2, Vector2, Y, y, FloatInst);

	Math.Vec3->operators.push_back({
		Operator::add,
		To.addFunction(NativeFunction({FunctionArgument(Math.Vec3, "a"), FunctionArgument(Math.Vec3, "b")},
			Math.Vec3, "Vector3.add", &Vector3_add))
		});

	Math.Vec3->operators.push_back({
		Operator::multiply,
		To.addFunction(NativeFunction({FunctionArgument(Math.Vec3, "a"), FunctionArgument(Math.Vec3, "b")},
			Math.Vec3, "Vector3.multiply", &Vector3_multiply))
		});

	Math.Vec3->operators.push_back({
		Operator::subtract,
		To.addFunction(NativeFunction({FunctionArgument(Math.Vec3, "a"), FunctionArgument(Math.Vec3, "b")},
			Math.Vec3, "Vector3.subtract", &Vector3_subtract))
		});

	Math.Vec3->operators.push_back({
		Operator::divide,
		To.addFunction(NativeFunction({FunctionArgument(Math.Vec3, "a"), FunctionArgument(Math.Vec3, "b")},
			Math.Vec3, "Vector3.divide", &Vector3_divide))
		});

	Math.Vec3->operators.push_back({
		Operator::unaryMinus,
		To.addFunction(NativeFunction({FunctionArgument(Math.Vec3, "a")},
			Math.Vec3, "Vector3.unaryMinus", &Vector3_unaryMinus))
		});

	To.addClassMethod(Math.Vec3, NativeFunction({},
		FloatInst, "length", &Vector3_length));

	To.addClassMethod(Math.Vec3, NativeFunction({ FunctionArgument(Math.Vec3, "other") },
		FloatInst, "dot", &Vector3_dot));

	To.addClassMethod(Math.Vec3, NativeFunction({ FunctionArgument(Math.Vec3, "other") },
		Math.Vec3, "cross", &Vector3_cross));

	To.addClassMethod(Math.Vec3, NativeFunction({},
		Math.Vec3, "normalized", &Vector3_normalized));

	To.addClassMethod(Math.Vec3, NativeFunction({},
		StrInst, "toString", &Vector3_toString));

	To.addClassMethod(Math.Vec3, NativeFunction({ FunctionArgument(Math.Vec3, "planeOrigin"),
		FunctionArgument(Math.Vec3, "planeNormal") },
		Math.Vec3, "projectToPlane", &Vector3_projectToPlane));

	auto Vec3Function = To.addFunction(
		NativeFunction({
			FunctionArgument(FloatInst, "x"),
			FunctionArgument(FloatInst, "y"),
			FunctionArgument(FloatInst, "z") },
			Math.Vec3, "vec3", [](InterpretContext* context) {}));

	Math.Vec3->addConstructor(Vec3Function);
	Math.Vec3->addConstructor(To.addFunction(
		NativeFunction({ FunctionArgument(FloatInst, "xyz") },
			Math.Vec3, "Vector3.new.xyz", [](InterpretContext* context) {
		float xyz = context->popValue<Float>();

		context->pushValue(Vector3(xyz));
	})));

	To.addClassMethod(Math.Rot,
		NativeFunction({ },
			Math.Vec3, "forward", &Rotation3_forward));

	To.addClassMethod(Math.Rot,
		NativeFunction({ },
			Math.Vec3, "right", &Rotation3_right));

	To.addClassMethod(Math.Rot,
		NativeFunction({ },
			Math.Vec3, "up", &Rotation3_up));

	To.addClassMethod(Math.Transform, NativeFunction({},
		DecomposeResultType, "decompose", &Transform_decompose));

	To.addClassMethod(Math.Transform, NativeFunction({ FunctionArgument(Math.Vec3, "value") },
		DecomposeResultType, "translate", &Transform_translate));

	To.addClassMethod(Math.Transform, NativeFunction({ FunctionArgument(Math.Rot, "value") },
		DecomposeResultType, "rotate", &Transform_rotate));

	To.addClassMethod(Math.Transform, NativeFunction({ FunctionArgument(Math.Rot, "axis"), FunctionArgument(FloatInst, "value") },
		DecomposeResultType, "rotate", &Transform_rotateAround));

	To.addClassMethod(Math.Transform, NativeFunction({ FunctionArgument(Math.Vec3, "value") },
		DecomposeResultType, "scale", Transform_scale));

	To.addClassConstructor(Math.Transform, NativeFunction({ },
		nullptr, "Transform.new.identity", Transform_identity));

	To.addClassConstructor(Math.Transform, NativeFunction({ FunctionArgument(Math.Vec3, "position"), FunctionArgument(Math.Rot, "rotation"), FunctionArgument(Math.Vec3, "scale") },
		nullptr, "Transform.new", Transform_new));

	auto Rot3Function = To.addFunction(
		NativeFunction({ FunctionArgument(FloatInst, "p"), FunctionArgument(FloatInst, "y"), FunctionArgument(FloatInst, "r") },
			Math.Rot, "rot3", [](InterpretContext* context) {}));

	Math.Rot->addConstructor(Rot3Function);

	auto Vec2Function = To.addFunction(
		NativeFunction({ FunctionArgument(FloatInst, "x"),FunctionArgument(FloatInst, "y") },
			Math.Vec2, "vec2", [](InterpretContext* context) {}));

	Math.Vec2->addConstructor(Vec2Function);

	To.addType(Math.Rot);
	To.addType(Math.Vec3);
	To.addType(Math.Vec2);
	To.addType(Math.Bounds);
	To.addType(Math.Transform);
	To.addType(DecomposeResultType);

	return Math;
}
