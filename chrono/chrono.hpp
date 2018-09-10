#pragma once
#include <chrono>
#include <unordered_map>

struct span
{
	std::chrono::steady_clock::time_point point;
	std::vector<long long> interval;
};
namespace TMR
{

	enum class time_type
	{
		nanoseconds,
		microseconds,
		milliseconds,
		seconds,
		minutes,
		hours
	};
	class TIMER
	{
	public:
		TIMER()
		{}
		void set(std::string name)
		{
			time[name] = std::chrono::steady_clock::now();
		}
		void start(std::string name)
		{
			time[name] = std::chrono::steady_clock::now();
		}
		void erasure(std::string name)
		{
			time.erase(name);
		}
    //Calculated interval
		unsigned long long calculate(std::string name_1, std::string name_2, time_type type = time_type::nanoseconds) 
		{
			switch (type)
			{
			case time_type::nanoseconds:
				return (time[name_1] > time[name_2] ? time[name_1] - time[name_2] : time[name_2] - time[name_1]).count();
				break;
			case time_type::microseconds:
				return ((time[name_1] > time[name_2] ? time[name_1] - time[name_2] : time[name_2] - time[name_1]) / 1000).count();
				break;
			case time_type::milliseconds:
				return ((time[name_1] > time[name_2] ? time[name_1] - time[name_2] : time[name_2] - time[name_1]) / 1000000).count();
				break;
			case time_type::seconds:
				return ((time[name_1] > time[name_2] ? time[name_1] - time[name_2] : time[name_2] - time[name_1]) / 1000000000).count();
				break;
			case time_type::minutes:
				return ((time[name_1] > time[name_2] ? time[name_1] - time[name_2] : time[name_2] - time[name_1]) / 1000000000 * 60).count();
				break;
			case time_type::hours:
				return ((time[name_1] > time[name_2] ? time[name_1] - time[name_2] : time[name_2] - time[name_1]) / 1000000000 * 60 * 60).count();
				break;
			default:
				return 0;
				break;
			}


		}
    //Save time interval
		void save_interval(std::string name)
		{
			if (time_array.find(name) == time_array.cend())
				return;
			time_array[name].interval.push_back((std::chrono::steady_clock::now()- time_array[name].point).count());

		}
    //Set a time starting point
		void set_time_point(std::string name)
		{
			span temp;
			temp.point = std::chrono::steady_clock::now();
			temp.interval.push_back(temp.point.time_since_epoch().count());
			time_array[name]= std::move(temp);
		}
    //Returns an interval array
		std::vector<long long> get(std::string name)
		{
			return time_array[name].interval;
		}

		~TIMER()
		{
			time.clear();
			time_array.clear();
		}

		
	private:
		std::unordered_map<std::string, std::chrono::steady_clock::time_point> time;
		std::unordered_map<std::string, span> time_array;
		

	};
}
