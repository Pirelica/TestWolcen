#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <cstdint>
#include <cmath>

using namespace std;

#define BRAKING_DISTANCE 1200

#define THRUST_MINIMUM 0
#define THRUST_MAXIMUM 100

#define PI 3.14159265f
#define DEG2RAD(angle) ((angle) * PI / 180.0f)
#define RAD2DEG(angle) ((angle) * 180.0f / PI)

//function to clamp a float value
float clip(float _n, float _lower, float _upper) 
{
	return max(_lower, min(_n, _upper));
}

#pragma region Vector2Class
class Vector2
{
private:
	float m_x;
	float m_y;
public:
	Vector2() = default;
	Vector2(float _x, float _y);
	~Vector2() = default;
	bool operator == (const Vector2& _v) const;
	bool operator != (const Vector2& _v) const;
	Vector2 operator -(const Vector2& _v);
	Vector2 operator *(const float _f);
	static float Dot(const Vector2& _v1, const Vector2& _v2);
	static float GetAngle(const Vector2& _vec1, const Vector2& _vec2);
	static float Length(const Vector2& _v);
	static Vector2 Normalize(const Vector2& _v);
	static Vector2 Rotate(const Vector2& _v, float angle);
	float GetX() { return m_x; }
	float GetY() { return m_y; }
};

Vector2::Vector2(float _x, float _y)
{
	m_x = _x;
	m_y = _y;
}

bool Vector2::operator==(const Vector2& _v) const
{
	return (m_x == _v.m_x && m_y == _v.m_y);
}

bool Vector2::operator!=(const Vector2& _v) const
{
	return (m_x != _v.m_x || m_y != _v.m_y);
}

Vector2 Vector2::operator-(const Vector2& _v)
{
	Vector2 result(m_x - _v.m_x, m_y - _v.m_y);
	return result;
}

float Vector2::Dot(const Vector2& _v1, const Vector2& _v2)
{
	return (_v1.m_x * _v2.m_x + _v1.m_y * _v2.m_y);
}

float Vector2::GetAngle(const Vector2& _v1, const Vector2& _v2)
{
	int angle = 0;
	angle = (int)acos(Dot(_v1, _v2));
	angle = RAD2DEG(angle);
	return angle;
}

float Vector2::Length(const Vector2& _v)
{
	return sqrt(pow(_v.m_x, 2) + pow(_v.m_y, 2));
}

Vector2 Vector2::Normalize(const Vector2& _v)
{
	if (Vector2::Length(_v) == 0.0f)
	{
		return _v;
	}
	else
	{
		Vector2 result(_v.m_x / Vector2::Length(_v), _v.m_y / Vector2::Length(_v));
		return result;
	}
}

Vector2 Vector2::Rotate(const Vector2& _v, float angle)
{
	float angleInRadian = angle * PI / 180;
	float angleSin = sin(angleInRadian);
	float angleCos = cos(angleInRadian);
	Vector2 result(_v.m_x * angleCos - _v.m_y * angleSin, _v.m_y * angleCos + _v.m_x * angleSin);
	return result;
}

Vector2 Vector2::operator*(const float _f)
{
	Vector2 result(m_x * _f, m_y * _f);
	return result;
}
#pragma endregion Vector2Class

#pragma region CheckpointManagerClass

class CheckpointManager
{
private:
	bool m_areAllCheckpointsFound = false;
	int m_biggestDistance;
	vector<Vector2> m_checkpoints;
	int m_currentCheckpoint = 0;
	int m_nbOfCheckpoints;

public:
	void AddNewCheckpoint(int _x, int _y, int _dist);
	bool IsFirstLapOver() const;
	int GetBiggestDistance() const;
};

void CheckpointManager::AddNewCheckpoint(int _x, int _y, int _dist)
{
	//avoid adding new checkpoints if the first lap is over
	if (m_areAllCheckpointsFound)
	{
		return;
	}
	Vector2 newCheckpoint(_x, _y);
	//adds the first checkpoint
	if (m_checkpoints.empty())
	{
		m_checkpoints.push_back(newCheckpoint);
		m_currentCheckpoint++;
	}
	else if (m_checkpoints.back() != newCheckpoint)
	{
		if (m_checkpoints.front() != newCheckpoint)
		{
			m_checkpoints.push_back(newCheckpoint);
			m_currentCheckpoint++;
		}
		else
		{
			m_areAllCheckpointsFound = true;
			m_nbOfCheckpoints = m_currentCheckpoint;
			m_currentCheckpoint = 0;
			m_checkpoints.pop_back();
		}
	}
	if (m_biggestDistance < _dist)
	{
		m_biggestDistance = _dist;
	}
}

bool CheckpointManager::IsFirstLapOver() const
{
	return m_areAllCheckpointsFound;
}

int CheckpointManager::GetBiggestDistance() const {
	return m_biggestDistance;
}

#pragma endregion CheckpointManagerClass

int main()
{
	float thrust = 100.0f;
	bool isBoosting = false;
	bool hasUsedBoost = false;
	CheckpointManager checkpointManager;
	// game loop
	while (1)
	{
		int x;
		int y;
		int nextCheckpointX; // x position of the next check point
		int nextCheckpointY; // y position of the next check point
		int nextCheckpointDist; // distance to the next checkpoint
		int nextCheckpointAngle; // angle between your pod orientation and the direction of the next checkpoint
		int opponentX;
		int opponentY;
		thrust = 100;
		cin >> x >> y >> nextCheckpointX >> nextCheckpointY >> nextCheckpointDist >> nextCheckpointAngle;
		cin.ignore();
		cin >> opponentX >> opponentY;
		cin.ignore();
		checkpointManager.AddNewCheckpoint(nextCheckpointX, nextCheckpointY, nextCheckpointDist);
		if (abs(nextCheckpointAngle) < 5)
		{
			cerr << "Small angle" << endl;
			thrust = THRUST_MAXIMUM;
			//conditions for the boost
			if (checkpointManager.IsFirstLapOver() && hasUsedBoost == false && nextCheckpointDist > 5000)
			{
				Vector2 playerToCheckpoint(nextCheckpointX - x, nextCheckpointY - y);
				Vector2 playerToOpponent(opponentX - x, opponentY - y);
				playerToCheckpoint = Vector2::Normalize(playerToCheckpoint);
				playerToOpponent = Vector2::Normalize(playerToOpponent);
				//verify that the opponent is not in front of me when I want to boost
				if (abs(Vector2::Dot(playerToCheckpoint, playerToOpponent)) < 0.8f)
				{
					isBoosting = true;
					hasUsedBoost = true;
				}
			}
			if (nextCheckpointDist < BRAKING_DISTANCE)
			{
				thrust = 100 * (nextCheckpointDist / BRAKING_DISTANCE) + 10.0f;
			}
		}
		else if (abs(nextCheckpointAngle) > 90)
		{
			thrust = 0;
		}
		else
		{
			//steering dehaviour
			Vector2 directionToCheckpoint(nextCheckpointX - x, nextCheckpointY - y);
			directionToCheckpoint = Vector2::Normalize(directionToCheckpoint);

			Vector2 currentDirection = Vector2::Rotate(directionToCheckpoint, -nextCheckpointAngle);
			currentDirection = Vector2::Normalize(currentDirection);

			Vector2 steering = (directionToCheckpoint - currentDirection);
			steering = Vector2::Normalize(steering) * 100;

			nextCheckpointX += steering.GetX();
			nextCheckpointY += steering.GetY();
			if (nextCheckpointDist < BRAKING_DISTANCE)
			{
				thrust = thrust * ((90.0f - abs(nextCheckpointAngle)) / 90.0f);
			}
		}

		if (isBoosting == true)
		{
			cout << nextCheckpointX << " " << nextCheckpointY << " " << "BOOST" << endl;
			isBoosting = false;
		}
		else
		{
			thrust = clip(thrust, 0, 100); // Clamp the trust to prevent < 0 or > 100
			cout << nextCheckpointX << " " << nextCheckpointY << " " << (int)thrust << " " << thrust << endl;
		}
	}
}

