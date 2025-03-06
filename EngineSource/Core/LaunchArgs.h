#pragma once
#include <Core/Types.h>
#ifndef ENGINE_CORE
#include <Engine/File/AssetRef.h>
#endif
#include <optional>
#include <vector>

/**
* @brief
* Engine launch argument functions.
*
* Namespace contains functions for reading launch arguments passed to the executable.
* After the launch arguments have been set using @SetArgs(), they can be read
* using the GetArg() functions.
*
* Example:
* Launch arguments: SomeExe -abc def -g
* -> Arguments: abc (with parameter "def"), g (with no parameters)
*
* An argument is everything that starts with a '-'.
* Anything after that argument that isn't a new argument is
* interpreted as parameters belonging to that argument.
*
* @ingroup engine-core
*/
namespace engine::launchArgs
{
	/**
	* @brief
	* Sets the launch arguments of this executable.
	*
	* The arguments should be passed directly from the main() function.
	*
	* @see engine::launchArgs
	*/
	void SetArgs(int argc, char** argv);

	/**
	* @brief
	* The parameter of a launch argument.
	*/
	struct Parameter
	{
		/**
		* @brief
		* Gets the value of this parameter as a string.
		*/
		string AsString() const;
#ifndef ENGINE_CORE
		AssetRef AsFile() const
		{
			return AssetRef::Convert(Value);
		}
#endif
		/**
		* @brief
		* Gets the value of this parameter as an int. Retruns 0 if it failed to parse.
		*/
		int32 AsInt() const;
		/**
		* @brief
		* Gets the value of this parameter as a float. Retruns 0 if it failed to parse.
		*/
		float AsFloat() const;

		friend void SetArgs(int argc, char** argv);

	private:
		string Value;
	};

	/**
	* Returns the parameters passed to a launch argument.
	*
	* If there were no parameters passed to the argument, the function returns an empty vector.
	*
	* If The argument doesn't exist, it returns an empty optional.
	*/
	std::optional<std::vector<Parameter>> GetArg(string Name);
}