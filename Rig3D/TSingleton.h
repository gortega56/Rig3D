#pragma once

namespace Rig3D
{
	template <class T>
	class TSingleton
	{
	public:

		static T& SharedInstance()
		{
			static T sharedInstance;
			return sharedInstance;
		}

		static void Destroy()
		{
			
		}

	private:
		TSingleton()
		{
			
		}

		~TSingleton()
		{
			
		}

		TSingleton(TSingleton<T> const&) = delete;
		void operator=(TSingleton<T> const&) = delete;
	};
}