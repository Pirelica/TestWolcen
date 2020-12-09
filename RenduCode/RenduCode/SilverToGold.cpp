#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <cstdint>
#include <cmath>

using namespace std;

#define CHECKPOINT_RADIUS 600
#define POD_RADIUS 400

#define BRAKING_DISTANCE 2400 //distance to slow down when approaching in a straight line
#define TURNING_DISTANCE 2400 //distance to start slowing down when using steering

#define STEERING_FACTOR 150.0f
#define BOOSTINGSAFEZONE 4000 //distance where the pod can boost without hitting the opponent

#define THRUST_MINIMUM 0.0f
#define THRUST_MAXIMUM 100.0f
#define THRUST_SLOW 10.0f

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
	float m_x = 0;
	float m_y = 0;
public:
	static float Dot(const Vector2& _v1, const Vector2& _v2);
	static float GetAngle(const Vector2& _vec1, const Vector2& _vec2);
	static float Length(const Vector2& _v);
	static Vector2 Normalize(const Vector2& _v);
	static Vector2 Rotate(const Vector2& _v, float angle);

	Vector2() = default;
	Vector2(float _x, float _y);
	~Vector2() = default;

	bool operator == (const Vector2& _v) const;
	bool operator != (const Vector2& _v) const;
	Vector2 operator -(const Vector2& _v);
	Vector2 operator *(const float _f);

	inline float GetX() { return m_x; }
	inline float GetY() { return m_y; }
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
	float angle = 0;
	angle = acos(Dot(_v1, _v2));
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
	float angleInRadian = angle * PI / 180.0f;
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
	int m_biggestDistance = 0;
	vector<Vector2> m_checkpoints;
	int m_currentCheckpoint = 0;
	int m_nbOfCheckpoints = 0;

public:
	void AddNewCheckpoint(const int _x, const int _y);
	inline bool IsFirstLapOver() const { return m_areAllCheckpointsFound; }
	inline int GetBiggestDistance() const { return m_biggestDistance; }
	void CheckBiggestDistance(const int _dist); //keep track of the biggest distance between checkpoints to use the boost at the best time
};

void CheckpointManager::AddNewCheckpoint(const int _x, const int _y)
{
	//avoid adding new checkpoints if the first lap is over
	if (m_areAllCheckpointsFound)
	{
		return;
	}
	Vector2 newCheckpoint((float)_x, (float)_y);
	//adds the first checkpoint
	if (m_checkpoints.empty())
	{
		m_checkpoints.push_back(newCheckpoint);
		m_currentCheckpoint++;
	}
	//adds the other checkpoints
	else if (m_checkpoints.back() != newCheckpoint)
	{
		//check that the next checkpoint is not the starting one
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
}

void CheckpointManager::CheckBiggestDistance(const int _dist)
{
	if (m_areAllCheckpointsFound)
	{
		return;
	}
	if (m_biggestDistance < _dist)
	{
		m_biggestDistance = _dist;
	}
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
		cin >> x >> y >> nextCheckpointX >> nextCheckpointY >> nextCheckpointDist >> nextCheckpointAngle;
		cin.ignore();
		cin >> opponentX >> opponentY;
		cin.ignore();
		cerr << "Distance = " << nextCheckpointDist << endl;
		int opponentDist = (int)sqrt((double)pow((opponentX - x), 2) + (double)pow((opponentY - y), 2));
		checkpointManager.AddNewCheckpoint(nextCheckpointX, nextCheckpointY);
		checkpointManager.CheckBiggestDistance(nextCheckpointDist);
		thrust = 100.0f;
		if (abs(nextCheckpointAngle) == 0)
		{
			cerr << "Small angle" << endl;
			thrust = THRUST_MAXIMUM;
			//conditions for the boost
			if (checkpointManager.IsFirstLapOver() && hasUsedBoost == false && nextCheckpointDist > 5000)
			{
				Vector2 playerToCheckpoint((float)(nextCheckpointX - x), (float)(nextCheckpointY - y));
				Vector2 playerToOpponent((float)(opponentX - x), (float)(opponentY - y));

				playerToCheckpoint = Vector2::Normalize(playerToCheckpoint);
				playerToOpponent = Vector2::Normalize(playerToOpponent);
				//verify that the opponent is not in front of me when I want to boost
				if ((abs(Vector2::Dot(playerToCheckpoint, playerToOpponent)) < 0.8f) || opponentDist < BOOSTINGSAFEZONE)
				{
					isBoosting = true;
					hasUsedBoost = true;
				}
			}
			//slow down depending on the distance when the checkpoint is close to prepare turning towards the next checkpoint
			if (nextCheckpointDist < BRAKING_DISTANCE)
			{
				cerr << "BRAKING_DISTANCE" << endl;
				thrust = 100.0f * ((float)nextCheckpointDist / (float)BRAKING_DISTANCE);
				clip(thrust, THRUST_SLOW, THRUST_MAXIMUM);
			}
		}
		else if (abs(nextCheckpointAngle) > 90)
		{
			thrust = 0.0f;
		}
		else
		{
			//steering behaviour
			Vector2 directionToCheckpoint((float)(nextCheckpointX - x), (float)(nextCheckpointY - y));
			directionToCheckpoint = Vector2::Normalize(directionToCheckpoint);

			Vector2 currentDirection = Vector2::Rotate(directionToCheckpoint, (float)-nextCheckpointAngle);
			currentDirection = Vector2::Normalize(currentDirection);

			Vector2 steering = (directionToCheckpoint - currentDirection);
			steering = Vector2::Normalize(steering) * STEERING_FACTOR;

			nextCheckpointX += (int)steering.GetX();
			nextCheckpointY += (int)steering.GetY();

			//slow down depending on the angle when the checkpoint is close to adjust my trajectory towards the checkpoint
			if (nextCheckpointDist < TURNING_DISTANCE)
			{
				cerr << "TURNING DISTANCE" << endl;
				thrust = thrust * ((90.0f - (float)abs(nextCheckpointAngle)) / 90.0f);
				clip(thrust, THRUST_SLOW, THRUST_MAXIMUM);
			}
		}

		if (isBoosting == true)
		{
			cout << nextCheckpointX << " " << nextCheckpointY << " " << "BOOST" << " " << "BOOST" << endl;
			isBoosting = false;
		}
		//Use shield if the pod is close to both the checkpoint and the opponent
		else if (opponentDist < POD_RADIUS * 2 && nextCheckpointDist < CHECKPOINT_RADIUS * 2)
		{
			cout << nextCheckpointX << " " << nextCheckpointY << " " << "SHIELD" << " " << "SHIELD" << endl;
		}
		else
		{
			//make sure that the thrust remains between 0 and 100
			thrust = clip(thrust, THRUST_MINIMUM, THRUST_MAXIMUM);
			cout << nextCheckpointX << " " << nextCheckpointY << " " << (int)thrust << " " << thrust << endl;
		}
	}
}