#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <iostream>
#include <cmath>
#include <string>
#include <vector>

using namespace std;

#define TIMEOUT_FIRST_TURN 500
#define TIMEOUT 75

#define SIMULATION_TURNS 4
#define SOLUTIONS_COUNT 6

#define THRUST_MAXIMUM 100
#define THRUST_BOOST 650

#define ROTATION_MAXIMUM 18

#define SHIELD_COOLDOWN 4

#define CHECKPOINT_RADIUS 600.0f
#define POD_RADIUS 400.0f

#define REBOUNCE_MINIMUM_IMPULSE 120.0f
#define FRICTION_FACTOR 0.85f

#define EPSILON 0.00001f
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
	friend class Simulation;
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

	Vector2(float _x, float _y) : m_x(_x), m_y(_y) {}
	Vector2() : m_x(0.f), m_y(0.f) {}
	~Vector2() = default;

	bool operator == (const Vector2& _v) const;
	bool operator != (const Vector2& _v) const;
	Vector2 operator -(const Vector2& _v);
	Vector2 operator *(const float _f);
	Vector2 operator + (const Vector2& _v);

	inline float GetX() { return m_x; }
	inline float GetY() { return m_y; }
};

Vector2 operator*(float k, Vector2& _v)
{
	return Vector2(k * _v.GetX(), k * _v.GetY());
}

Vector2 operator+=(Vector2& _v1, const Vector2& _v2)
{
	_v1 = _v1 + _v2;
}

Vector2 operator*=(Vector2& _v, float _f)
{
	_v = _f * _v;
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

struct Pod
{
	Vector2 position;
	Vector2 speed;
	int angle = -1;

	int nextCheckpointId = 0;
	int totalCheckpointsPassed = 0;

	bool hasBoosted = false;
	int shieldCooldown = 0;

	int score = 0;
};

void ManageShield(bool _isTurnedOn, Pod _pod)
{
	if (_isTurnedOn)
	{
		_pod.shieldCooldown = SHIELD_COOLDOWN;
	}
	else if (_pod.shieldCooldown > 0)
	{
		_pod.shieldCooldown--;
	}
}

void UpdatePodInfo(Pod& _pod)
{
	int x, y, speedX, speedY, angle, nextCheckPointId;
	cin >> x >> y >> speedX >> speedY >> angle >> nextCheckPointId;
	cin.ignore();

	_pod.position = Vector2((float)x, (float)y);
	_pod.speed = Vector2((float)speedX, (float)speedY);
	_pod.angle = angle;
	//check if the pod has passed a checkpoint since the last turn
	if (_pod.nextCheckpointId != nextCheckPointId)
	{
		_pod.totalCheckpointsPassed++;
	}
	_pod.nextCheckpointId = nextCheckPointId;
}
#pragma region PhysicsFunctions
float GetPodMass(const Pod& _pod)
{
	float mass = 1.0f;
	//check if shield is active
	if (_pod.shieldCooldown == SHIELD_COOLDOWN)
	{
		mass = 10.0f;
	}
	return mass;
}

float TimeToCollision(Pod& _pod1, Pod& _pod2)
{
	//physics simulation to check if a collision is imminent
	Vector2 positionDifference = _pod2.position - _pod1.position;
	Vector2 speedDifference = _pod2.speed - _pod1.speed;

	float a = Vector2::Dot(speedDifference, speedDifference);
	if (a < EPSILON)
	{
		return INFINITY;
	}

	float b = -2.0f * Vector2::Dot(positionDifference, speedDifference);
	float c = Vector2::Dot(positionDifference, positionDifference) - 4.0f * pow(POD_RADIUS, 2);

	float delta = b * b - 4.f * a * c;
	if (delta < 0.0f)
	{
		return INFINITY;
	}

	float time = (b - sqrt(delta)) / (2.f * a);
	if (time <= EPSILON)
	{
		return INFINITY;
	}
	return time;
}

void Rebounce(Pod& _podA, Pod& _podB)
{
	//calculate how the pods will rebounce after a collision
	float massA = GetPodMass(_podA);
	float massB = GetPodMass(_podB);

	Vector2 positionDifference = (_podB.position - _podA.position);
	float distance = Vector2::Distance(_podA.position, _podB.position);

	Vector2 dirVec = positionDifference * (1.0f / distance);
	Vector2 speedDifference = (_podB.speed - _podA.speed);

	float mass = (massA * massB) / (massA + massB);
	float dirDotSpeedDiff = Vector2::Dot(speedDifference, dirVec);

	float impulse = -2.0f * mass * dirDotSpeedDiff;
	impulse = clip(impulse, -REBOUNCE_MINIMUM_IMPULSE, REBOUNCE_MINIMUM_IMPULSE);

	_podA.speed += (-1.0f * massA) * impulse * dirVec;
	_podB.speed += (1.0f * massB) * impulse * dirVec;
}
#pragma endregion PhysicsFunctions

#pragma region BaseSimulationData
struct Move
{
	int rotation = 0; // between -ROTATION_MAXIMUM and ROTATION_MAXIMUM
	int thrust = 0; //between 0 and THRUST_MAXIMUM
	bool useBoost = false;
	bool useShield = false;
};

class Turn
{
private:
	vector<Move> m_moves = vector<Move>(2);
public:
	Move& operator[](size_t m) { return m_moves[m]; }
	const Move& operator[](size_t m) const { return m_moves[m]; }
};

class Solution
{
private:
	vector<Turn> m_turns = vector<Turn>(SIMULATION_TURNS);
public:
	Turn& operator[](size_t t) { return m_turns[t]; }
	const Turn& operator[](size_t t) const { return m_turns[t]; }

	int score = -1;
};
#pragma endregion BaseSimulationData

#pragma region SimulationClass
class Simulation
{
private:
	vector<Vector2> m_checkpoints;
	int m_checkpointCount; //checkpoints in one lap
	int m_maxCheckpoints; //total of checkpoints in all of the laps
public:
	int GetMaxCheckpoints() const { return m_maxCheckpoints; }
	const vector<Vector2>& GetCheckpoints() const { return m_checkpoints; }
	Vector2 InitCheckpoints();
	void ComputeSolution(vector<Pod>& pods, const Solution& _solution) const;
private:
	void ComputeRotation(vector<Pod>& pods, const Turn& turn) const;
	void computeSpeed(vector<Pod>& pods, const Turn& turn) const;
	void ApplyRotationAndThrust(vector<Pod>& pods) const;
	void ApplyFriction(vector<Pod>& pods) const;
	void FinishTurn(vector<Pod>& pods) const;
	void ComputeWholeTurn(vector<Pod>& pods, const Turn& turn) const;
};

Vector2 Simulation::InitCheckpoints()
{
	int laps;
	cin >> laps;
	cin.ignore();
	cin >> m_checkpointCount;
	cin.ignore();
	m_checkpoints.reserve(m_checkpointCount);
	for (int i = 0; i < m_checkpointCount; i++)
	{
		int x, y;
		cin >> x >> y;
		cin.ignore();
		m_checkpoints[i] = Vector2((float)x, (float)y);
	}
	m_maxCheckpoints = m_checkpointCount * laps;
	//return the first checkpoint that the pods will have to reach
	return m_checkpoints[1];
}

void Simulation::ComputeSolution(vector<Pod>& _pods, const Solution& _solution) const
{
	for (int i = 0; i < SIMULATION_TURNS; i++)
	{
		ComputeWholeTurn(_pods, _solution[i]);
	}
}
//expert rule number 1
void Simulation::ComputeRotation(vector<Pod>& _pods, const Turn& _turn) const
{
	for (int i = 0; i < 2; i++)
	{
		Pod& pod = _pods[i];
		const Move& move = _turn[i];

		pod.angle = (pod.angle + move.rotation) % 360;
	}
}
//expert rule number 2
void Simulation::computeSpeed(vector<Pod>& _pods, const Turn& _turn) const
{
	for (int i = 0; i < 2; i++)
	{
		Pod& pod = _pods[i];
		const Move& move = _turn[i];

		ManageShield(move.useShield, pod);
		if (pod.shieldCooldown > 0)
		{
			continue;
		}

		float angleRad = DEG2RAD(pod.angle);
		Vector2 direction(cos(angleRad), sin(angleRad));

		bool useBoost = false;
		if (!pod.hasBoosted && move.useBoost)
		{
			useBoost = true;
		}
		int thrust;
		if (useBoost)
		{
			thrust = THRUST_BOOST;
			pod.hasBoosted = true;
		}
		else
		{
			thrust = move.thrust;
		}
		pod.speed += (float)thrust * direction;
	}
}
//expert rule number 3
void Simulation::ApplyRotationAndThrust(vector<Pod>& _pods) const
{
	float time = 0.0f;
	float endTime = 1.0f;
	while (time < endTime)
	{
		//Check for collisions
		Pod* podA = nullptr;
		Pod* podB = nullptr;
		float dt = endTime - time;
		for (int i = 0; i < 4; i++)
		{
			for (int j = i + 1; j < 4; j++)
			{
				float collisionTime = TimeToCollision(_pods[i], _pods[j]);
				if ((time + collisionTime < endTime) && (collisionTime < dt))
				{
					dt = collisionTime;
					podA = &_pods[i];
					podB = &_pods[j];
				}
			}
		}
		//check collisions with checkpoints
		for (Pod& pod : _pods)
		{
			pod.position += dt * pod.speed;

			if (pow(Vector2::Distance(pod.position, m_checkpoints[pod.nextCheckpointId]), 2) < pow(CHECKPOINT_RADIUS, 2))
			{
				pod.nextCheckpointId = (pod.nextCheckpointId + 1) % m_checkpointCount;
				pod.totalCheckpointsPassed++;
			}
		}
		if (podA != nullptr && podB != nullptr)
		{
			Rebounce(*podA, *podB);
		}
		time += dt;
	}
}
//expert rule number 4
void Simulation::ApplyFriction(vector<Pod>& _pods) const
{
	for (Pod& pod : _pods)
	{
		pod.speed *= FRICTION_FACTOR;
	}
}
//expert rule number 5
void Simulation::FinishTurn(vector<Pod>& _pods) const
{
	for (Pod& pod : _pods)
	{
		pod.speed = Vector2{ round(pod.speed.m_x), round(pod.speed.m_y) };
		pod.position = Vector2{ round(pod.position.GetX()), round(pod.position.GetY()) };
	}
}

void Simulation::ComputeWholeTurn(vector<Pod>& _pods, const Turn& _turn) const
{
	//Application of the "expert rules"
	ComputeRotation(_pods, _turn);
	computeSpeed(_pods, _turn);
	ApplyRotationAndThrust(_pods);
	ApplyFriction(_pods);
	FinishTurn(_pods);
}
#pragma endregion SimulationClass

void OutputSolution(const Solution& _solution, vector<Pod>& _pods)
{
	constexpr int turn = 0;
	for (int i = 0; i < 2; i++)
	{
		Pod& pod = _pods[i];
		const Move& move = _solution[turn][i];

		float angle = (pod.angle + (float)move.rotation) % 360;
		float angleRad = DEG2RAD(angle);

		constexpr float targetDistance = 10000.0f;
		Vector2 direction{ targetDistance * cos(angleRad), targetDistance * sin(angleRad) };
		Vector2 target = pod.position + direction;

		cout << round(target.GetX()) << " " << round(target.GetY()) << " ";
		if (move.useShield)
		{
			cout << "SHIELD" << " SHIELD";
		}
		else if (move.useBoost)
		{
			cout << "BOOST" << " BOOST";
		}
		else
		{
			cout << move.thrust << " " << move.thrust;
		}
		cout << endl;
	}
}

void UpdateShieldAndBoostForNextTurn(const Solution& _solution, vector<Pod>& _pods)
{
	for (int i = 0; i < 2; i++)
	{
		Pod& pod = _pods[i];
		const Move& move = _solution[0][i];

		ManageShield(move.useShield, pod);
		if (pod.shieldCooldown == 0 && move.useBoost)
		{
			pod.hasBoosted = true;
		}
	}
}

//Much faster function than std::rand to generate a random number
int fastrand()
{
	static unsigned int g_seed = 100;
	g_seed = (214013 * g_seed + 2531011);
	return (g_seed >> 16) & 0x7FFF;
}
inline int rnd(int a, int b)
{
	return (fastrand() % (b - a)) + a;
}

#pragma region SolverClass
class Solver
{
private:
	vector<Solution> m_solutions;
	Simulation* m_simulation;

public:
	Solver(Simulation* _simulation);
	const Solution& Solve(const vector<Pod>& _pods, int _time);

private:
	void InitPopulation();
	void FirstTurnBoost();
	void Randomize(Move& _move, bool _modifyAll = true) const;
	void ShiftByOneTurn(Solution& _solution) const;
	void Mutate(Solution& _solution) const;
	int ComputeScore(Solution& _solution, const vector<Pod>& _pods) const;
	int RateSolution(vector<Pod>& _pods) const;
};

Solver::Solver(Simulation* _simulation)
{
	m_simulation = _simulation;
	InitPopulation();
	FirstTurnBoost();
}

const Solution& Solver::Solve(const vector<Pod>& _pods, int _time)
{
	//check if we can keep iterating without spending too much time
	using namespace std::chrono;
	auto start = high_resolution_clock::now();
	//check if I have enough time to run another round of solutions
	auto keepSolving = [&start, _time]() -> bool
	{
		auto now = high_resolution_clock::now();
		auto d = duration_cast<milliseconds>(now - start);
		return (d.count() < _time);
	};
	//init this turn
	for (int i = 0; i < SOLUTIONS_COUNT; i++)
	{
		Solution& s = m_solutions[i];
		ShiftByOneTurn(s);
		ComputeScore(s, _pods);
	}

	while (keepSolving())
	{

		//build and rate mutated versions of our solutions
		for (int i = 0; i < SOLUTIONS_COUNT; ++i)
		{
			Solution& newSolution = m_solutions[SOLUTIONS_COUNT + i];
			newSolution = m_solutions[i];
			Mutate(newSolution);
			ComputeScore(newSolution, _pods);
		}
		//sort the solutions by score
		std::sort(m_solutions.begin(), m_solutions.end(), [](const Solution& a, const Solution& b)
			{return a.score > b.score; });
	}
	return m_solutions[0];
}

void Solver::InitPopulation()
{
	// 0 to (SOLUTIONS_COUNT - 1) are actual solutions from the previous turn
	// SOLUTIONS_COUNT to (2 * SOLUTIONS_COUNT - 1): temporary solutions from Solve()
	m_solutions.resize(2 * SOLUTIONS_COUNT);

	//randomize starting solutions
	for (int s = 0; s < SOLUTIONS_COUNT; s++)
	{
		for (int t = 0; t < SIMULATION_TURNS; t++)
		{
			for (int i = 0; i < 2; i++)
			{
				Randomize(m_solutions[s][t][i]);
			}
		}
	}
}
//Try to use the boost on the first turn
void Solver::FirstTurnBoost()
{
	float distance = pow(Vector2::Distance(m_simulation->GetCheckpoints()[0], m_simulation->GetCheckpoints()[1]), 2);
	float boostDistanceThreshold = 9000000.f;
	if (distance < boostDistanceThreshold)
	{
		return;
	}
	for (int i = 0; i < 2; i++)
	{
		for (int s = 0; s < SOLUTIONS_COUNT; s++)
		{
			m_solutions[s][0][i].useBoost = true;
		}
	}
}
//modify one or all of the values of a move
void Solver::Randomize(Move& _move, bool _modifyAll) const
{
	constexpr int all = -1, rotation = 0, thrust = 1, shield = 2, boost = 3;
	constexpr int probRotation = 5, probThrust = probRotation + 5, probShield = probThrust + 1, probBoost = probShield + 0;
	int i;
	if (_modifyAll)
	{
		i = -1;
	}
	else
	{
		i = rnd(0, probBoost);
	}
	const int valueToModify = [i, _modifyAll]() -> int
	{
		if (_modifyAll)
			return true;
		if (i <= probRotation)
			return rotation;
		if (i <= probThrust)
			return thrust;
		if (i <= probShield)
			return shield;
		return boost;
	}();
	auto modifyValue = [valueToModify](int parValue)
	{
		if (valueToModify == all)
			return true;
		return valueToModify == parValue;
	};
	if (modifyValue(rotation))
	{
		// arbitrarily give more weight to -ROTATION_MAXIMUM, 0, ROTATION_MAXIMUM
		const int r = rnd(-2 * ROTATION_MAXIMUM, 3 * ROTATION_MAXIMUM);
		if (r > 2 * ROTATION_MAXIMUM)
		{
			_move.rotation = 0;
		}
		else
		{
			_move.rotation = clamp(r, -ROTATION_MAXIMUM, ROTATION_MAXIMUM);
		}
	}
	if (modifyValue(thrust))
	{
		// arbitrarily give more weight to 0, THRUST_MAXIMUM
		const int r = rnd(-THRUST_MAXIMUM / 2, 2 * THRUST_MAXIMUM);
		_move.thrust = clamp(r, 0, THRUST_MAXIMUM);
	}
	if (modifyValue(shield))
	{
		if (!_modifyAll || (rnd(0, 10) > 6))
		{
			cerr << "Shield on" << endl;
			_move.useShield = !_move.useShield;
		}
	}
	if (modifyValue(boost))
	{
		if (!_modifyAll || (rnd(0, 10) > 6))
		{
			_move.useBoost = !_move.useBoost;
		}
	}
}

void Solver::ShiftByOneTurn(Solution& _solution) const
{
	for (int t = 1; t < SIMULATION_TURNS; t++)
	{
		for (int i = 0; i < 2; i++)
		{
			_solution[t - 1][i] = _solution[t][i];
		}
	}
	//create a new random turn
	for (int i = 0; i < 2; i++)
	{
		Move& move = _solution[SIMULATION_TURNS - 1][i];
		Randomize(move);
	}
}

void Solver::Mutate(Solution& _solution) const
{
	//mutate one value with a random t,i
	int k = rnd(0, 2 * SIMULATION_TURNS);
	Move& move = _solution[k / 2][k % 2];

	Randomize(move, false);
}

int Solver::ComputeScore(Solution& _solution, const vector<Pod>& _pods) const
{
	vector<Pod> podsCopy = _pods;
	m_simulation->ComputeSolution(podsCopy, _solution);
	_solution.score = RateSolution(podsCopy);
	return _solution.score;
}

int Solver::RateSolution(vector<Pod>& _pods) const
{
	//get the score of each pod
	auto podScore = [&](const Pod& _pod) -> int
	{
		constexpr int cpFactor = 30000;
		const int distToCp = (int)Vector2::Distance(_pod.position, m_simulation->GetCheckpoints()[_pod.nextCheckpointId]);
		return cpFactor * _pod.totalCheckpointsPassed - distToCp;
	};
	for (Pod& pod : _pods)
	{
		pod.score = podScore(pod);
	}

	int myRacerIndex;
	//check which of my pods is ahead of the other
	if (_pods[0].score > _pods[1].score)
	{
		myRacerIndex = 0;
	}
	else
	{
		myRacerIndex = 1;
	}
	Pod& myRacer = _pods[myRacerIndex];
	Pod& myInterceptor = _pods[1 - myRacerIndex];

	//check which of the opponent pods is ahead of the other
	Pod opponentRacer;
	if (_pods[2].score > _pods[3].score)
	{
		opponentRacer = _pods[2];
	}
	else
	{
		opponentRacer = _pods[3];
	}

	if (myRacer.totalCheckpointsPassed > m_simulation->GetMaxCheckpoints())
	{
		return (int)INFINITY; //Victory!
	}
	if (opponentRacer.totalCheckpointsPassed > m_simulation->GetMaxCheckpoints())
	{
		return (int)INFINITY; //Defeat!
	}

	//score difference between my racer and the opponent racer
	const int aheadScore = myRacer.score - opponentRacer.score;

	//check if my interceptor can block the opponent racer or his destination checkpoint
	Vector2 opponentCheckpoint = m_simulation->GetCheckpoints()[opponentRacer.nextCheckpointId];
	int interceptorScore;
	if (myRacer.nextCheckpointId == opponentRacer.nextCheckpointId)
	{
		interceptorScore = (int)-Vector2::Distance(myInterceptor.position, opponentRacer.position);
	}
	else
	{
		interceptorScore = (int)-Vector2::Distance(myInterceptor.position, opponentCheckpoint);
	}

	constexpr int aheadBias = 2; // being ahead is better than blocking the opponent
	return aheadScore * aheadBias + interceptorScore;
}
#pragma endregion SolverClass

//makes pods face the checkpoint on the first turn
void OverrideAngle(Pod& _pod, Vector2& _target)
{
	Vector2 dir = Vector2::Normalize(_target - _pod.position);
	float a = acos(dir.GetX()) * 180.f / PI;
	if (dir.GetY() < 0)
	{
		a = (360.f - a);
	}
	_pod.angle = (int)a;
}

int main()
{
	Simulation simulation;
	Vector2 firstCheckpoint = simulation.InitCheckpoints();
	Solver solver{ &simulation };
	vector<Pod> pods(4);
	int step = 0;
	while (1)
	{
		for (int i = 0; i < 4; i++)
		{
			UpdatePodInfo(pods[i]);
			if (step == 0)
			{
				OverrideAngle(pods[i], firstCheckpoint);
			}
		}
		int availableTime = 0;
		if (step == 0)
		{
			availableTime = TIMEOUT_FIRST_TURN;

		}
		else
		{
			availableTime = TIMEOUT;
		}
		float timeoutSafeGuard = 0.95f;

		const Solution& solution = solver.Solve(pods, (int)(availableTime * timeoutSafeGuard));
		OutputSolution(solution, pods);
		UpdateShieldAndBoostForNextTurn(solution, pods);
		++step;
	}
}