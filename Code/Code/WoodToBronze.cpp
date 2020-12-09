#include <iostream>
#include <string>
#include <vector>
#include <algorithm>

using namespace std;

/**
 * Auto-generated code below aims at helping you parse
 * the standard input according to the problem statement.
 **/

int main()
{

    // game loop
    while (1) {
        int x;
        int y;
        int nextCheckpointX; // x position of the next check point
        int nextCheckpointY; // y position of the next check point
        int nextCheckpointDist; // distance to the next checkpoint
        int nextCheckpointAngle; // angle between your pod orientation and the direction of the next checkpoint
        cin >> x >> y >> nextCheckpointX >> nextCheckpointY >> nextCheckpointDist >> nextCheckpointAngle; cin.ignore();
        int opponentX;
        int opponentY;
        int thrust;
        bool canBoost = true;
        bool isBoosting = false;
        cin >> opponentX >> opponentY; cin.ignore();

        if (nextCheckpointAngle > -10 && nextCheckpointAngle < 10 && nextCheckpointDist > 5000 && canBoost == true)
        {
            isBoosting = true;
            canBoost = false;
        }
        if (nextCheckpointAngle > 90 || nextCheckpointAngle < -90)
        {
            thrust = 0;
        }
        else if (nextCheckpointDist < 1000)
        {
            thrust = 20;
        }
        else
        {
            thrust = 100;
        }

        // Write an action using cout. DON'T FORGET THE "<< endl"
        // To debug: cerr << "Debug messages..." << endl;


        // You have to output the target position
        // followed by the power (0 <= thrust <= 100)
        // i.e.: "x y thrust"
        if (isBoosting)
        {
            cout << nextCheckpointX << " " << nextCheckpointY << " " << "BOOST" << endl;
        }
        else
        {
            cout << nextCheckpointX << " " << nextCheckpointY << " " << thrust << endl;
        }
    }
}