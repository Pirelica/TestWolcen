#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <cstdint>
#include <cmath>

using namespace std;

#define CHECKPOINT_RADIUS 600
#define POD_RADIUS 400

#define BRAKING_DISTANCE 1800 //distance to slow down when approaching in a straight line
#define TURNING_DISTANCE 2400 //distance to start slowing down when using steering

#define STEERING_FACTOR 150.0f
#define BOOSTINGSAFEZONE 4000 //distance where the pod can boost without hitting the opponent

#define THRUST_MINIMUM 0.0f
#define THRUST_MAXIMUM 100
#define THRUST_SLOW 10.0f

#define SHIELD_COOLDOWN 4 // turn of activation + 3 turns of inactivity

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
	friend class Pod;
private:
	float m_x = 0;
	float m_y = 0;
public:
	static float Dot(const Vector2& _v1, const Vector2& _v2);
	static float GetAngle(const Vector2& _vec1, const Vector2& _vec2);
	static float Length(const Vector2& _v);
	static Vector2 Normalize(const Vector2& _v);
	static Vector2 Rotate(const Vector2& _v, float angle);
	static float Distance(const Vector2& _v1, const Vector2& _v2);

	Vector2() = default;
	Vector2(float _x, float _y);
	~Vector2() = default;

	bool operator == (const Vector2& _v) const;
	bool operator != (const Vector2& _v) const;
	Vector2 operator -(const Vector2& _v);
	Vector2 operator *(const float _f);
	Vector2 operator + (const Vector2& _v);

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

float Vector2::Distance(const Vector2& _v1, const Vector2& _v2)
{
	return sqrt(pow(_v2.m_x - _v1.m_x, 2) + pow(_v2.m_y - _v1.m_y, 2));
}

Vector2 Vector2::operator*(const float _f)
{
	Vector2 result(m_x * _f, m_y * _f);
	return result;
}

Vector2 Vector2::operator+(const Vector2& _v)
{
	Vector2 result(m_x + _v.m_x, m_y + _v.m_y);
	return result;
}
#pragma endregion Vector2Class
struct Checkpoint
{
	int x;
	int y;
};
#pragma region PodClass

class Pod
{
private:
	friend class Vector2;
	Vector2 m_pos;
	Vector2 m_speed;
	Vector2 m_destination;
	int m_angle;
	int m_nextCheckpointId;
	int m_angleToCheckpoint;
	int m_thrust;
	Checkpoint m_nextCheckpoint;
	bool m_isBoosting;
	bool m_hasUsedBoost;
	bool m_isShielding;
	int m_shieldCooldown = 0;
	int m_checkpointsPassed = 0;
	bool m_isRacer = true;
	float m_distanceToCheckpoint;

public:
	inline void SetAngle(int _angle) { m_angle = _angle; }
	inline void SetNextCheckpoint(int _cp) { m_nextCheckpointId = _cp; }
	inline void SetAngleToCheckpoint(int _angle) { m_angleToCheckpoint = _angle; }
	inline int GetCheckpointId() { return m_nextCheckpointId; }
	inline bool GetIsRacer() { return m_isRacer; }

	void UpdateInfo(Checkpoint* _checkpoints);
	void UpdateAngleToCheckpoint(Checkpoint _cp);
	void UpdateSteering();
	void UpdateThrust(Pod* enemies, int _target, Checkpoint* _checkpoints, int _cpNb);
	void GiveOutput();
	void ComputeShield(Pod* _myPods, Pod* _opponents, int _index);
	static void GetFirstPod(Pod* _myPods);
};

void Pod::UpdateInfo(Checkpoint* _checkpoints)
{
	int checkpointId;
	cin >> m_pos.m_x >> m_pos.m_y >> m_speed.m_x >> m_speed.m_y >> m_angle >> checkpointId;
	if (checkpointId != m_nextCheckpointId)
	{
		m_nextCheckpointId = checkpointId;
		m_checkpointsPassed++;
	}
	cin.ignore();
	Vector2 checkpointPos((float)_checkpoints[m_nextCheckpointId].x, (float)_checkpoints[m_nextCheckpointId].y);
	m_distanceToCheckpoint = Vector2::Distance(m_pos, checkpointPos);
	m_destination.m_x = (float)_checkpoints[m_nextCheckpointId].x;
	m_destination.m_y = (float)_checkpoints[m_nextCheckpointId].y;
}

void Pod::GetFirstPod(Pod* _myPods)
{
	if (_myPods[0].m_checkpointsPassed > _myPods[1].m_checkpointsPassed)
	{
		_myPods[0].m_isRacer = true;
		_myPods[1].m_isRacer = false;
	}
	else if (_myPods[1].m_checkpointsPassed > _myPods[0].m_checkpointsPassed)
	{
		_myPods[0].m_isRacer = false;
		_myPods[1].m_isRacer = true;
	}
	else
	{
		if (_myPods[0].m_distanceToCheckpoint > _myPods[1].m_distanceToCheckpoint)
		{
			_myPods[0].m_isRacer = false;
			_myPods[1].m_isRacer = true;
		}
		else if (_myPods[1].m_distanceToCheckpoint > _myPods[0].m_distanceToCheckpoint)
		{
			_myPods[0].m_isRacer = true;
			_myPods[1].m_isRacer = false;
		}
	}
}

void Pod::UpdateAngleToCheckpoint(Checkpoint _cp)
{
	Vector2 podToCheckpoint(_cp.x - m_pos.m_x, _cp.y - m_pos.m_y);
	podToCheckpoint = Vector2::Normalize(podToCheckpoint);
	float a = acos(podToCheckpoint.m_x);
	RAD2DEG(a);
	//a -= (float)m_angle;
	m_angleToCheckpoint = (int)(abs(a));
	cerr << "Angle = " << m_angleToCheckpoint << endl;
	
}

void Pod::ComputeShield(Pod* _myPods, Pod* _opponents, int _index)
{
	if (m_shieldCooldown == 0)
	{
		if ((int)Vector2::Distance(_myPods[0].m_pos, _myPods[1].m_pos) <= POD_RADIUS * 2)
		{
			m_isShielding = true;
			m_shieldCooldown = SHIELD_COOLDOWN;
		}
		for (int i = 0; i < 2; i++)
		{
			if (Vector2::Distance(_myPods[i].m_pos + _myPods[i].m_speed, _opponents[i].m_pos + _opponents[i].m_speed) <= POD_RADIUS * 2)
			{
				m_isShielding = true;
				m_shieldCooldown = SHIELD_COOLDOWN;
			}
		}
	}
	else
	{
		m_shieldCooldown--;
		cerr << "Shield cd = " << m_shieldCooldown << endl;
	}
}

void Pod::UpdateSteering()
{
	if (abs(m_angleToCheckpoint) < 90)
	{
		//steering behaviour
		Vector2 directionToCheckpoint((float)(m_destination.m_x - m_pos.m_x), (float)(m_destination.m_y - m_pos.m_x));
		directionToCheckpoint = Vector2::Normalize(directionToCheckpoint);

		Vector2 currentDirection = Vector2::Rotate(directionToCheckpoint, (float)-m_angleToCheckpoint);
		currentDirection = Vector2::Normalize(currentDirection);

		Vector2 steering = (directionToCheckpoint - currentDirection);
		steering = Vector2::Normalize(steering) * STEERING_FACTOR;

		m_destination.m_x += (int)steering.GetX();
		m_destination.m_y += (int)steering.GetY();
	}
}

void Pod::UpdateThrust(Pod* enemies, int _target, Checkpoint* _checkpoints, int _cpNb)
{
	if (!m_isRacer)
	{
		Vector2 checkpointDest((float)_checkpoints[enemies[_target].m_nextCheckpointId].x, (float)_checkpoints[enemies[_target].m_nextCheckpointId].y);
		m_destination = checkpointDest;
	}

	m_thrust = THRUST_MAXIMUM;
	int checkpointDist = (int)sqrt(pow(m_destination.m_x - m_pos.m_x, 2) + pow(m_destination.m_y - m_pos.m_y, 2));
	if (m_isRacer)
	{
		if (abs(m_angleToCheckpoint) == 0)
		{
			m_thrust = THRUST_MAXIMUM;
			//conditions for the boost
			if (m_hasUsedBoost == false && checkpointDist > 5000)
			{
				bool isBoostingSafe = true;
				Vector2 playerToCheckpoint((float)(m_destination.m_x - m_pos.m_x), (float)(m_destination.m_y - m_pos.m_y));
				playerToCheckpoint = Vector2::Normalize(playerToCheckpoint);
				for (int i = 0; i < 2; i++)
				{
					Vector2 playerToOpponent((float)(enemies[i].m_pos.m_x - m_pos.m_x), (float)(enemies[i].m_pos.m_y - m_pos.m_y));
					int opponentDist = (int)(Vector2::Length(playerToOpponent));
					playerToOpponent = Vector2::Normalize(playerToOpponent);
					if (abs(Vector2::Dot(playerToCheckpoint, playerToOpponent)) > 0.8f && opponentDist < BOOSTINGSAFEZONE)
					{
						isBoostingSafe = false;
					}
					if (isBoostingSafe)
					{
						m_isBoosting = true;
						m_hasUsedBoost = true;
					}

				}
			}
		}
		//slow down depending on the distance when the checkpoint is close to prepare turning towards the next checkpoint
		if (checkpointDist < BRAKING_DISTANCE)
		{
			if (m_nextCheckpointId == _cpNb - 1)
			{
				Vector2 newDestination((float)_checkpoints[0].x, (float)_checkpoints[0].y);
				m_destination = newDestination;
			}
			else
			{
				Vector2 newDestination((float)_checkpoints[m_nextCheckpointId + 1].x, (float)_checkpoints[m_nextCheckpointId + 1].y);
				m_destination = newDestination;
			}
			m_thrust = 50;
		}
	}
	else if (abs(m_angleToCheckpoint) > 90)
	{
		cerr << "HARD TURN = " << m_angleToCheckpoint << endl;
		m_thrust = 0;
	}
	else
	{
		//slow down depending on the angle when the checkpoint is close to adjust my trajectory towards the checkpoint

		cerr << "STEERING TURN" << endl;
		m_thrust = m_thrust * ((90 - abs(m_angleToCheckpoint)) / 90) - 10;
		m_thrust = clamp(m_thrust, 10, THRUST_MAXIMUM);

	}
}

void Pod::GiveOutput()
{
	if (m_isBoosting)
	{
		cout << m_destination.m_x << " " << m_destination.m_y << " " << "BOOST" << " BOOST" << endl;
		m_isBoosting = false;
	}
	else if (m_isShielding)
	{
		cout << m_destination.m_x << " " << m_destination.m_y << " " << "SHIELD" << " SHIELD" << endl;
		m_isShielding = false;
	}
	else
	{
		cout << m_destination.m_x << " " << m_destination.m_y << " " << m_thrust << " " << m_thrust << endl;
	}
}

#pragma endregion PodClass

int main()
{
	//new inputs from the gold league
	int laps;
	int checkpointCount;
	cin >> laps >> checkpointCount;
	cin.ignore();
	Checkpoint _checkpoints[checkpointCount];
	int target; //index of the first opponent pod in the race
	for (int i = 0; i < checkpointCount; i++)
	{
		cin >> _checkpoints[i].x >> _checkpoints[i].y;
		cin.ignore();
	}
	Pod myPods[2];
	Pod opponentPods[2];
	// game loop
	while (1)
	{
		for (int i = 0; i < 2; i++)
		{
			myPods[i].UpdateInfo(_checkpoints);

		}
		Pod::GetFirstPod(myPods);
		Pod::GetFirstPod(opponentPods);
		if (opponentPods[0].GetIsRacer())
		{
			target = 0;
		}
		else
		{
			target = 1;
		}
		for (int i = 0; i < 2; i++)
		{
			opponentPods[i].UpdateInfo(_checkpoints);
		}

		for (int i = 0; i < 2; i++)
		{
			myPods[i].UpdateAngleToCheckpoint(_checkpoints[myPods[i].GetCheckpointId()]);
			myPods[i].ComputeShield(myPods, opponentPods, i);
			myPods[i].UpdateSteering();
			myPods[i].UpdateThrust(opponentPods, target, _checkpoints, checkpointCount);
			myPods[i].GiveOutput();
		}
	}
}