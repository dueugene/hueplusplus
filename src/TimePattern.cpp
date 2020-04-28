/**
    \file TimePattern.cpp
    Copyright Notice\n
    Copyright (C) 2020  Jan Rogall		- developer\n

    This file is part of hueplusplus.

    hueplusplus is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    hueplusplus is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with hueplusplus.  If not, see <http://www.gnu.org/licenses/>.
**/

#include <ctime>

#include <hueplusplus/HueExceptionMacro.h>
#include <hueplusplus/TimePattern.h>

namespace hueplusplus
{
namespace time
{

using clock = std::chrono::system_clock;
std::string timepointToTimestamp(clock::time_point time)
{
    using namespace std::chrono;
    std::time_t ctime = clock::to_time_t(time);

    std::tm localtime = *std::localtime(&ctime);
    char buf[32];

    std::size_t result = std::strftime(buf, sizeof(buf), "%FT%T", &localtime);
    if (result == 0)
    {
        throw HueException(CURRENT_FILE_INFO, "strftime failed");
    }
    return std::string(buf);
}

clock::time_point parseTimestamp(const std::string& timestamp)
{
    std::tm tm;
    tm.tm_year = std::stoi(timestamp.substr(0, 4));
    tm.tm_mon = std::stoi(timestamp.substr(5, 2));
    tm.tm_mday = std::stoi(timestamp.substr(8, 2));
    tm.tm_hour = std::stoi(timestamp.substr(11, 2));
    tm.tm_min = std::stoi(timestamp.substr(14, 2));
    tm.tm_sec = std::stoi(timestamp.substr(17, 2));
    // Auto detect daylight savings time
    tm.tm_isdst = -1;
    std::time_t ctime = std::mktime(&tm);
    return clock::from_time_t(ctime);
}

std::string durationTo_hh_mm_ss(clock::duration duration)
{
    using namespace std::chrono;
    if (duration > hours(24))
    {
        throw HueException(CURRENT_FILE_INFO, "Duration parameter longer than 1 day");
    }
    int numH = static_cast<int>(duration_cast<hours>(duration).count());
    duration -= hours(numH);
    int numM = static_cast<int>(duration_cast<minutes>(duration).count());
    duration -= minutes(numM);
    int numS = static_cast<int>(duration_cast<seconds>(duration).count());

    char result[9];
    std::sprintf(result, "%02d:%02d:%02d", numH, numM, numS);
    return std::string(result);
}

clock::duration parseDuration(const std::string& s)
{
    using namespace std::chrono;
    return hours(std::stoi(s.substr(0, 2))) + minutes(std::stoi(s.substr(3, 2))) + seconds(std::stoi(s.substr(7, 2)));
}

AbsoluteTime::AbsoluteTime(clock::time_point baseTime, clock::duration variation) : base(baseTime), variation(variation)
{}

clock::time_point AbsoluteTime::getBaseTime()
{
    return base;
}
clock::duration AbsoluteTime::getRandomVariation()
{
    return variation;
}
std::string AbsoluteTime::toString()
{
    std::string result = timepointToTimestamp(base);
    if (variation.count() != 0)
    {
        result.push_back('A');
        result.append(durationTo_hh_mm_ss(variation));
    }
    return result;
}

bool Weekdays::isNone() const
{
    return bitmask == 0;
}

bool Weekdays::isAll() const
{
    // Check all 7 bits are set
    return bitmask == (1 << 7) - 1;
}

bool Weekdays::isMonday() const
{
    return (bitmask & 1) != 0;
}

bool Weekdays::isTuesday() const
{
    return (bitmask & 2) != 0;
}

bool Weekdays::isWednesday() const
{
    return (bitmask & 4) != 0;
}

bool Weekdays::isThursday() const
{
    return (bitmask & 8) != 0;
}

bool Weekdays::isFriday() const
{
    return (bitmask & 16) != 0;
}

bool Weekdays::isSaturday() const
{
    return (bitmask & 32) != 0;
}

bool Weekdays::isSunday() const
{
    return (bitmask & 64) != 0;
}

std::string Weekdays::toString() const
{
    std::string result = std::to_string(bitmask);
    if (result.size() < 3)
    {
        result.insert(0, 3 - result.size(), '0');
    }
    return result;
}

Weekdays Weekdays::unionWith(Weekdays other) const
{
    other.bitmask |= bitmask;
    return other;
}

Weekdays Weekdays::none()
{
    return Weekdays();
}

Weekdays Weekdays::all()
{
    Weekdays result;
    result.bitmask = (1 << 7) - 1;
    return result;
}

Weekdays Weekdays::monday()
{
    return Weekdays(0);
}

Weekdays Weekdays::tuesday()
{
    return Weekdays(1);
}

Weekdays Weekdays::wednesday()
{
    return Weekdays(2);
}

Weekdays Weekdays::thursday()
{
    return Weekdays(3);
}

Weekdays Weekdays::friday()
{
    return Weekdays(4);
}

Weekdays Weekdays::saturday()
{
    return Weekdays(5);
}

Weekdays Weekdays::sunday()
{
    return Weekdays(6);
}

Weekdays Weekdays::parse(const std::string& s)
{
    Weekdays result;
    result.bitmask = std::stoi(s);
    return result;
}

RecurringTime::RecurringTime(clock::duration daytime, Weekdays days, clock::duration variation)
    : time(daytime), days(days), variation(variation)
{}

clock::duration RecurringTime::getDaytime() const
{
    return time;
}

clock::duration RecurringTime::getRandomVariation() const
{
    return variation;
}

Weekdays RecurringTime::getWeekdays() const
{
    return days;
}

std::string RecurringTime::toString() const
{
    std::string result = "W";
    result.append(days.toString());
    result.append("/");
    result.append(durationTo_hh_mm_ss(time));
    if (variation.count() != 0)
    {
        result.push_back('A');
        result.append(durationTo_hh_mm_ss(variation));
    }
    return std::string();
}

TimeInterval::TimeInterval(clock::duration start, clock::duration end, Weekdays days)
    : start(start), end(end), days(days)
{}

clock::duration TimeInterval::getStartTime() const
{
    return start;
}

clock::duration TimeInterval::getEndTime() const
{
    return end;
}

Weekdays TimeInterval::getWeekdays() const
{
    return days;
}

std::string TimeInterval::toString() const
{
    std::string result;
    if (!days.isAll())
    {
        result.append("W");
        result.append(days.toString());
        result.append("/");
    }
    result.push_back('T');
    result.append(durationTo_hh_mm_ss(start));
    result.append("/T");
    result.append(durationTo_hh_mm_ss(end));

    return result;
}

Timer::Timer(clock::duration duration, clock::duration variation)
    : expires(duration), numExecutions(1), variation(variation)
{}

Timer::Timer(clock::duration duration, int numExecutions, clock::duration variation)
    : expires(duration), numExecutions(numExecutions), variation(variation)
{}

bool Timer::isRecurring() const
{
    return numExecutions != 1;
}

int Timer::getNumberOfExecutions() const
{
    return numExecutions;
}

clock::duration Timer::getExpiryTime() const
{
    return expires;
}

clock::duration Timer::getRandomVariation() const
{
    return variation;
}

std::string Timer::toString() const
{
    std::string result;
    if (numExecutions != 1)
    {
        result.push_back('R');
        if (numExecutions != 0)
        {
            std::string s = std::to_string(numExecutions);
            // Pad to two digits
            if (s.size() < 2)
            {
                result.push_back('0');
            }
            result.append(s);
        }
        result.push_back('/');
    }
    result.append("PT");
    result.append(durationTo_hh_mm_ss(expires));
    if (variation.count() != 0)
    {
        result.push_back('A');
        result.append(durationTo_hh_mm_ss(variation));
    }
    return result;
}

TimePattern::TimePattern() : type(Type::undefined), undefined(nullptr) {}

TimePattern::~TimePattern()
{
    destroy();
}

TimePattern::TimePattern(const AbsoluteTime& absolute) : type(Type::absolute)
{
    new (&this->absolute) AbsoluteTime(absolute);
}

TimePattern::TimePattern(const RecurringTime& recurring) : type(Type::recurring)
{
    new (&this->recurring) RecurringTime(recurring);
}

TimePattern::TimePattern(const TimeInterval& interval) : type(Type::interval)
{
    new (&this->interval) TimeInterval(interval);
}

TimePattern::TimePattern(const Timer& timer) : type(Type::timer)
{
    new (&this->timer) Timer(timer);
}

TimePattern::TimePattern(const TimePattern& other) : type(Type::undefined), undefined(nullptr)
{
    *this = other;
}

TimePattern& TimePattern::operator=(const TimePattern& other)
{
    if (this == &other)
    {
        return *this;
    }
    destroy();
    try
    {
        type = other.type;
        switch (type)
        {
        case Type::undefined:
            undefined = nullptr;
            break;
        case Type::absolute:
            new (&absolute) AbsoluteTime(other.absolute);
            break;
        case Type::recurring:
            new (&recurring) RecurringTime(other.recurring);
            break;
        case Type::interval:
            new (&interval) TimeInterval(other.interval);
            break;
        case Type::timer:
            new (&timer) Timer(other.timer);
            break;
        }
    }
    catch (...)
    {
        // Catch any throws from constructors to stay in valid state
        type = Type::undefined;
        undefined = nullptr;
        throw;
    }
    return *this;
}

TimePattern::Type TimePattern::getType() const
{
    return type;
}

AbsoluteTime TimePattern::asAbsolute() const
{
    return absolute;
}

RecurringTime TimePattern::asRecurring() const
{
    return recurring;
}

TimeInterval TimePattern::asInterval() const
{
    return interval;
}

Timer TimePattern::asTimer() const
{
    return timer;
}

TimePattern TimePattern::parse(const std::string& s)
{
    if (s.empty() || s == "none")
    {
        return TimePattern();
    }
    else if (std::isdigit(s.front()))
    {
        // Absolute time
        clock::time_point time = parseTimestamp(s);
        clock::duration variation {0};
        if (s.size() > 19 && s[19] == 'A')
        {
            // Random variation
            variation = parseDuration(s.substr(20));
        }
        return TimePattern(AbsoluteTime(time, variation));
    }
    else if (s.front() == 'R' || s.front() == 'P')
    {
        // (Recurring) timer
        int numRepetitions = 1;
        if (s.front() == 'R')
        {
            if (s.at(1) == '/')
            {
                // Infinite
                numRepetitions = -1;
            }
            else
            {
                numRepetitions = std::stoi(s.substr(1, 2));
            }
        }
        std::size_t start = s.find('T') + 1;
        std::size_t randomStart = s.find('A');
        clock::duration expires = parseDuration(s.substr(start, randomStart - start));
        clock::duration variance = std::chrono::seconds(0);
        if (randomStart != std::string::npos)
        {
            variance = parseDuration(s.substr(randomStart));
        }
        return TimePattern(Timer(expires, numRepetitions, variance));
    }
    else if (s.front() == 'W' && std::count(s.begin(), s.end(), '/') == 1)
    {
        // Recurring time
        Weekdays days = Weekdays::parse(s.substr(1, 3));
        clock::duration time = parseDuration(s.substr(6));
        clock::duration variation {0};
        if (s.size() > 14)
        {
            variation = parseDuration(s.substr(15));
        }
        return TimePattern(RecurringTime(time, days, variation));
    }
    else if (s.front() == 'T' || s.front() == 'W')
    {
        Weekdays days = Weekdays::all();
        if (s.front() == 'W')
        {
            // Time interval with weekdays
            days = Weekdays::parse(s.substr(1, 3));
        }
        // Time interval
        std::size_t start = s.find('T') + 1;
        std::size_t end = s.find('/', start);
        clock::duration startTime = parseDuration(s.substr(start, end - start));
        clock::duration endTime = parseDuration(s.substr(end + 2));
        return TimePattern(TimeInterval(startTime, endTime, days));
    }
    throw HueException(CURRENT_FILE_INFO, "Unable to parse time string: " + s);
}

void TimePattern::destroy()
{
    switch (type)
    {
    case Type::absolute:
        absolute.~AbsoluteTime();
        break;
    case Type::recurring:
        recurring.~RecurringTime();
        break;
    case Type::interval:
        interval.~TimeInterval();
        break;
    case Type::timer:
        timer.~Timer();
        break;
    default:
        break;
    }
    type = Type::undefined;
    undefined = nullptr;
}

} // namespace time
} // namespace hueplusplus