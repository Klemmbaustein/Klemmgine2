#include "MathBindings.h"
#include <Core/Vector.h>
#include <Core/Transform.h>

using namespace ds;
using namespace engine;

static void Vector3_Forward(InterpretContext* context)
{
	context->pushValue(Vector3::Forward(context->popValue<Rotation3>().EulerVector()));
}

static void Vector3_Right(InterpretContext* context)
{
	context->pushValue(Vector3::Right(context->popValue<Rotation3>().EulerVector()));
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

static void Vector3_UnaryMinus(InterpretContext* context)
{
	context->pushValue(-context->popValue<Vector3>());
}

static void Vector3_length(InterpretContext* context)
{
	context->pushValue(context->popValue<Vector3>().Length());
}

script::MathBindings engine::script::AddMathModule(ds::NativeModule& To)
{
	script::MathBindings Math;

	auto FloatInst = FloatType::getInstance();

	Math.Vec3 = DS_CREATE_STRUCT(Vector3);

	DS_STRUCT_MEMBER_NAME(Math.Vec3, Vector3, X, x, FloatInst);
	DS_STRUCT_MEMBER_NAME(Math.Vec3, Vector3, Y, y, FloatInst);
	DS_STRUCT_MEMBER_NAME(Math.Vec3, Vector3, Z, z, FloatInst);

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
			Math.Vec3, "Vector3.unaryMinus", &Vector3_UnaryMinus))
		});

	To.addStructMethod(Math.Vec3, NativeFunction({},
		FloatInst, "length", &Vector3_length));

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

	Math.Rot = DS_CREATE_STRUCT(Rotation3);

	DS_STRUCT_MEMBER_NAME(Math.Rot, Rotation3, P, p, FloatInst);
	DS_STRUCT_MEMBER_NAME(Math.Rot, Rotation3, Y, y, FloatInst);
	DS_STRUCT_MEMBER_NAME(Math.Rot, Rotation3, R, r, FloatInst);

	To.addFunction(
		NativeFunction({ FunctionArgument(Math.Rot, "rotation") },
			Math.Vec3, "forward", &Vector3_Forward));

	To.addFunction(
		NativeFunction({ FunctionArgument(Math.Rot, "rotation") },
			Math.Vec3, "right", &Vector3_Right));

	auto Rot3Function = To.addFunction(
		NativeFunction({ FunctionArgument(FloatInst, "p"),FunctionArgument(FloatInst, "y"),FunctionArgument(FloatInst, "r") },
			Math.Rot, "rot3", [](InterpretContext* context) {}));

	Math.Rot->addConstructor(Rot3Function);

	Math.Vec2 = DS_CREATE_STRUCT(Vector2);

	DS_STRUCT_MEMBER_NAME(Math.Vec2, Vector2, X, x, FloatInst);
	DS_STRUCT_MEMBER_NAME(Math.Vec2, Vector2, Y, y, FloatInst);

	auto Vec2Function = To.addFunction(
		NativeFunction({ FunctionArgument(FloatInst, "x"),FunctionArgument(FloatInst, "y") },
			Math.Vec2, "vec2", [](InterpretContext* context) {}));

	Math.Vec2->addConstructor(Vec2Function);

	To.types.push_back(Math.Rot);
	To.types.push_back(Math.Vec3);
	To.types.push_back(Math.Vec2);

	return Math;
}
