#include "PhysicsManager.h"
#include "SoundParams.h"
#include "PlayerManager.h"

const float PhysicsManager::VELOCITY_CAP = 80.0f;
const float player_speed = 10.0f;
const float player_jump_speed = 20.0f;
const float ROT_CAP = 700.0f;
const unsigned int JUMP_SOUND_PERIOD = 29;

PhysicsManager::PhysicsManager(std::vector<Planetoid> *p, std::vector<Core> *c, Map *m)
{
	map = m;
	cores = c;
	planets = p;
	objects = std::vector<PhysicsObject*>();
    frames_since_jump_sound = 0;
}

void PhysicsManager::calcPhysics(float dTime)
{
	//do physics on each object
	for (size_t i = 0; i < objects.size(); i++)
	{
		PhysicsObject *object = objects[i];
		glm::vec2 pos = object->get_position();
		glm::vec3 rot = object->get_rotation();
		glm::vec2 vel = object->velocity;
		glm::vec2 max_acc;
		glm::vec2 acc = gravAcceleration(pos, max_acc);
		pos += vel * dTime;
		vel += acc * dTime;

		// player movement
		bool player_input = false;
		Character *character = dynamic_cast<Character *>(object);
		if (character && character->controllable && character->grounded) {
			glm::vec2 N_acc = -glm::normalize(max_acc);
			glm::vec2 T_acc = { N_acc.y, -N_acc.x };

			float N_comp = dot(N_acc, vel);
			float T_comp = dot(T_acc, vel);

			if (character->jump_input) {
                //Section that ensures Jump sounds have delay to avoid cacophony
                if (frames_since_jump_sound > JUMP_SOUND_PERIOD) {
                    frames_since_jump_sound = 0;
                    //standard template for sending a sound effect event
                    //make parameter object
                    SoundParams * JumpNoise = new SoundParams();

                    //Define sound to be played
                    JumpNoise->sound = Jump;

                    //define sound position (0's as place holders, should be the position of where this is called)
                    JumpNoise->x = 0;
                    JumpNoise->y = 0;
                    JumpNoise->z = 0;
                    
                    //Cast Sound Parameter to a Type Param
                    TypeParam<SoundParams*> *jumpSound = new TypeParam<SoundParams*>(JumpNoise);
                    //fire notification with the type Param
                    EventManager::notify(PLAY_SOUND, jumpSound);
                }
				player_input = true;
				N_comp = player_jump_speed;
			}
		   	if (character->left_input || character->right_input) {
				player_input = true;
				T_comp = player_speed * (character->right_input - character->left_input);
				rot.y = 90 - 90.0f * (character->right_input - character->left_input);
			}

			vel = N_acc * N_comp + T_acc * T_comp;
		}

		// collision response
		glm::vec2 N_col;
		size_t colliding = object->colliding_with_map(*map, pos, max_acc, N_col);
		glm::vec2 T_col = { N_col.y, -N_col.x };
		if (colliding == 4) {
			// if glitching into ground, don't
			pos = (pos == object->get_position()) ? pos - glm::normalize(max_acc) : object->get_position();
			vel = glm::vec2(0);
		} else if (colliding) {
			float N_comp = dot(N_col, vel);
			float T_comp = dot(T_col, vel);

			if (N_comp < 0) {
				N_comp = 0;
			}
			if (!player_input) {
				T_comp = 0;
			}

			vel = N_col * N_comp + T_col * T_comp;
		}

		// clamp velocity magnitude
		if (length(vel) > VELOCITY_CAP) {
			vel = glm::normalize(vel) * VELOCITY_CAP;
		}

		float rot_desired = atan2(max_acc.x, -max_acc.y) * 180.0f / M_PI;
		float rot_diff = normalizeAngle(rot_desired - rot.z);
		float rot_diff_clamped = std::max<float>(std::min<float>(rot_diff, ROT_CAP * dTime), -ROT_CAP * dTime);
		rot.z += rot_diff_clamped;

		object->set_position(pos);
		object->set_rotation(rot);
		object->velocity = vel;
		object->grounded = colliding;
	}

	for (int i = 0; i < cores->size(); i++) {
		Core* core = &(*cores)[i];
		for (int j = 0; j < PlayerManager::instance->players.size(); j++) {
			Player* player = PlayerManager::instance->players[j];
			for (int k = 0; k < player->chars.size(); k++) {
				Character *character = player->chars[k];
				glm::vec2 normal;
				if (core->colliding_with_object(*character, normal)) {
					std::cout << "Player burned up. \n";
					PlayerManager::instance->players[j]->RemoveCharacter(k);
				}
			}
		}
	}

    frames_since_jump_sound++;
}

//Finds the net force on a given point in space
glm::vec2 PhysicsManager::gravAcceleration(glm::vec2 pos, glm::vec2 &max_acceleration)
{
	glm::vec2 net_acceleration = glm::vec2(0, 0);
	float max_acceleration_magnitude = 0;

	//calculate the force of each planetoid and add them to the vector
	for (Planetoid p : *planets)
	{
		//calculate distance between planet and object
		glm::vec2 direction = p._pos - pos;
		float distance = glm::length(direction);

		//calculate the force from each planetoid
		float magnitude = p._m / (distance * distance);
		glm::vec2 acceleration = glm::normalize(direction) * magnitude;

		//add force to the total force
		if (magnitude > max_acceleration_magnitude) {
			max_acceleration = acceleration;
			max_acceleration_magnitude = magnitude;
		}

		net_acceleration += acceleration;
	}

	return net_acceleration;
}

// takes angle in degrees and normalizes it to a range of -180 to 180
float PhysicsManager::normalizeAngle(float angle) {
	angle = fmod(angle + 180.0f, 360.0f);
	if (angle < 0) {
		angle += 360.0f;
	}
	return angle - 180.0f;
}


void PhysicsManager::addObject(PhysicsObject *obj)
{
	objects.push_back(obj);
}

glm::vec2 PhysicsManager::genSpawnPos(float object_radius)
{
	int *pRands = new int[planets->size()];
	int sum = 0;

	for (int i = 0; i < planets->size(); i++)
	{
		sum += (*planets)[i]._r;
		pRands[i] = sum;
	}

	int rand = std::rand() % sum;
	
	for (int i = 0; i < planets->size(); i++)
	{
		if (rand < pRands[i])
		{
			delete pRands;
			return posOnPlanet(i, object_radius);
		}
	}
}

// gets a random position on the planetoid's surface
glm::vec2 PhysicsManager::posOnPlanet(int pInd, float object_radius)
{
	float radius = (*planets)[pInd]._r + object_radius;
	float angle = rand() / (RAND_MAX / (2.0f * M_PI));
	float x = std::cos(angle)*radius + (*planets)[pInd]._x;
	float y = std::sin(angle)*radius + (*planets)[pInd]._y;
	return glm::vec2(x, y);
}