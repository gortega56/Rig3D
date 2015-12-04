#include <Windows.h>
#include "Rig3D\Engine.h"
#include "Rig3D\Graphics\Interface\IScene.h"
#include "Rig3D\Graphics\DirectX11\DX3D11Renderer.h"
#include "Rig3D\Graphics\Interface\IMesh.h"
#include "Rig3D\Common\Transform.h"
#include "Memory\Memory\Memory.h"
#include "Rig3D\Graphics\MeshLibrary.h"
#include "Rig3D\Graphics\Interface\IShader.h"
#include "Rig3D\Graphics\Interface\IShaderResource.h"
#include "Rig3D/Intersection.h"
#include "Rig3D/Visibility.h"
#include "Rig3D/Graphics/Camera.h"
#include <d3d11.h>
#include <ctime>
#include <random>

#define RAND(range, offset)					(std::rand() % range) - offset

#define PI							3.1415926535f
#define CAMERA_SPEED				0.1f
#define CAMERA_ROTATION_SPEED		0.1f
#define RADIAN						3.1415926535f / 180.0f
#define INSTANCE_COUNT				4
#define SCENE_MEMORY				10240
#define BOID_COLLIDER_RADIUS		1.5f
#define OBSTACLE_AVOIDANCE_WEIGHT	0.2f
#define BOID_INVERSE_MASS			0.2f
#define PHYSICS_TIME_STEP			0.001f			// ms
#define MAX_ACCELERATION_MAGNITUDE  2.0f

using namespace Rig3D;

static float gSeparationWeight	= 0.5f;
static float gAlignmentWeight	= 0.5f;
static float gCohesionWeight	= 0.5f;

uint8_t gSceneMemory[SCENE_MEMORY];

struct Vertex3
{
	vec3f Position;
	vec3f Normal;
	vec2f UV;
};

struct RigidBody
{
	vec3f velocity;
	vec3f forces;
};

struct Boid
{
	vec3f			nPositions[3];
	vec3f			nDirections[3];
	float			nDistances[3];
	uint32_t		nIndices[3];
	
	Transform*		transform;
	RigidBody*		rigidBody;
	SphereCollider* collider;
};

struct ViewProjection
{
	mat4f view;
	mat4f projection;
};

class GroupMotionSample : public IScene, public IRendererDelegate
{
public:
	ViewProjection		mViewProjection;
	Camera				mCamera;
	Plane<vec3f>		mPlanes[6];
	float				mMouseX;
	float				mMouseY;

	LinearAllocator		mLinearAllocator;
	
	mat4f*				mBoidWorldMatrices;
	
	Transform*			mBoidTransforms;
	RigidBody*			mBoidRigidBodies;
	SphereCollider*		mBoidColliders;
	Boid*				mBoids;

	IRenderer*			mRenderer;
	IShader*			mBoidVertexShader;
	IShader*			mBoidPixelShader;
	IShaderResource*	mBoidShaderResouce;
	IMesh*				mBoidMesh;

	GroupMotionSample();
	~GroupMotionSample();

	void VInitialize() override;
	void VUpdate(double milliseconds) override;
	void VRender() override;
	void VShutdown() override;

	void VOnResize() override;

	void InitializeGeometry();
	void InitializePhysics();
	void InitializeShaders();
	void InitializeShaderResources();

	void UpdateShaderResources();
	void UpdateInput(Input& input);
	void UpdateCamera();

	void UpdateBoidNeighbors();
	void UpdateBoidBehaviors();
	void UpdateBoidForces();
	
	void IntegrateBoids(float deltaTime);
	void ResetBoids();
};

GroupMotionSample::GroupMotionSample() : 
	mMouseX(0.0f),
	mMouseY(0.0f),
	mLinearAllocator(gSceneMemory, gSceneMemory + SCENE_MEMORY),
	mBoidWorldMatrices(nullptr),
	mBoidTransforms(nullptr),
	mBoidRigidBodies(nullptr),
	mBoidColliders(nullptr),
	mBoids(nullptr),
	mRenderer(nullptr),
	mBoidVertexShader(nullptr),
	mBoidPixelShader(nullptr),
	mBoidShaderResouce(nullptr),
	mBoidMesh(nullptr)
{
	mOptions.mWindowCaption = "Group Motion Sample";
	mOptions.mWindowWidth = 1200;
	mOptions.mWindowHeight = 900;
	mOptions.mGraphicsAPI = GRAPHICS_API_DIRECTX11;
	mOptions.mFullScreen = false;
}

GroupMotionSample::~GroupMotionSample()
{
	
}

void GroupMotionSample::VInitialize()
{
	mRenderer = &DX3D11Renderer::SharedInstance();
	mRenderer->SetDelegate(this);

	time_t now;
	time(&now);
	srand(static_cast<unsigned>(now));

	InitializeGeometry();
	InitializePhysics();
	InitializeShaders();
	InitializeShaderResources();
	VOnResize();
}

void GroupMotionSample::InitializeGeometry()
{
	OBJBasicResource<Vertex3> boidResource("Models\\boid.obj");

	MeshLibrary<LinearAllocator> meshLibrary;
	meshLibrary.SetAllocator(&mLinearAllocator);
	meshLibrary.LoadMesh(&mBoidMesh, mRenderer, boidResource);

	mCamera.mTransform.SetPosition({ 0.0f, 0.0f, -20.0f });

	mPlanes[0].normal = vec3f(0.0f, 0.0f, -1.0f);	// Front
	mPlanes[1].normal = vec3f(0.0f, 0.0f, 1.0f);	// Back
	mPlanes[2].normal = vec3f(-1.0f, 0.0f, 0.0f);	// Left
	mPlanes[3].normal = vec3f(1.0f, 0.0f, 0.0f);	// Right
	mPlanes[4].normal = vec3f(0.0f, 1.0f, 0.0f);	// Top
	mPlanes[5].normal = vec3f(0.0f, -1.0f, 0.0f);	// Bottom

	for (int i = 0; i < 6; i++)
	{
		mPlanes[i].distance = 20.0f;
	}
}

void GroupMotionSample::InitializePhysics()
{
	mBoidWorldMatrices	= reinterpret_cast<mat4f*>(mLinearAllocator.Allocate(sizeof(mat4f) * INSTANCE_COUNT, alignof(mat4f), 0));
	mBoidTransforms		= reinterpret_cast<Transform*>(mLinearAllocator.Allocate(sizeof(Transform) * INSTANCE_COUNT, alignof(Transform), 0));
	mBoidRigidBodies	= reinterpret_cast<RigidBody*>(mLinearAllocator.Allocate(sizeof(RigidBody) * INSTANCE_COUNT, alignof(RigidBody), 0));
	mBoidColliders		= reinterpret_cast<SphereCollider*>(mLinearAllocator.Allocate(sizeof(SphereCollider) * INSTANCE_COUNT, alignof(SphereCollider), 0));
	mBoids				= reinterpret_cast<Boid*>(mLinearAllocator.Allocate(sizeof(Boid) * INSTANCE_COUNT, alignof(Boid), 0));

	ResetBoids();
}

void GroupMotionSample::InitializeShaders()
{
	mRenderer->VCreateShader(&mBoidVertexShader, &mLinearAllocator);
	mRenderer->VCreateShader(&mBoidPixelShader, &mLinearAllocator);

	InputElement boidInputElements[] =
	{
		{ "POSITION",	0, 0, 0, 0, RGB_FLOAT32, INPUT_CLASS_PER_VERTEX },
		{ "NORMAL",		0, 0, 12, 0, RGB_FLOAT32, INPUT_CLASS_PER_VERTEX },
		{ "TEXCOORD",	0, 0, 24, 0, RG_FLOAT32, INPUT_CLASS_PER_VERTEX },
		{ "WORLD",		0, 1, 0, 1, RGBA_FLOAT32, INPUT_CLASS_PER_INSTANCE },
		{ "WORLD",		1, 1, 16, 1, RGBA_FLOAT32, INPUT_CLASS_PER_INSTANCE },
		{ "WORLD",		2, 1, 32, 1, RGBA_FLOAT32, INPUT_CLASS_PER_INSTANCE },
		{ "WORLD",		3, 1, 48, 1, RGBA_FLOAT32, INPUT_CLASS_PER_INSTANCE }
	};

	mRenderer->VLoadVertexShader(mBoidVertexShader, "BoidVertexShader.cso", boidInputElements, 7);
	mRenderer->VLoadPixelShader(mBoidPixelShader, "BoidPixelShader.cso");
}

void GroupMotionSample::InitializeShaderResources()
{
	mRenderer->VCreateShaderResource(&mBoidShaderResouce, &mLinearAllocator);

	void*	instanceData[]			= { &mBoidWorldMatrices };
	size_t	instanceDataSizes[]		= { sizeof(mat4f) * INSTANCE_COUNT };
	size_t	instanceDataStrides[]	= { sizeof(mat4f) };
	size_t	instanceDataOffsets[]	= { 0 };

	mRenderer->VCreateDynamicShaderInstanceBuffers(mBoidShaderResouce, instanceData, instanceDataSizes, instanceDataStrides, instanceDataOffsets, 1);

	void*	constantData[]		= { &mViewProjection };
	size_t	constantDataSize[]	= { sizeof(ViewProjection) };

	mRenderer->VCreateShaderConstantBuffers(mBoidShaderResouce, constantData, constantDataSize, 1);
}

void GroupMotionSample::UpdateCamera()
{
	mCamera.SetViewMatrix(mat4f::lookAtLH(mCamera.mTransform.GetForward(), mCamera.mTransform.GetPosition(), vec3f(0.0f, 1.0f, 0.0f)));
}

void GroupMotionSample::VUpdate(double milliseconds)
{
	UpdateInput(Input::SharedInstance());
	UpdateCamera();

	UpdateBoidNeighbors();
	UpdateBoidBehaviors();
	UpdateBoidForces();
	IntegrateBoids(static_cast<float>(milliseconds) * 0.001f);	// Convert to seconds

	UpdateShaderResources();	

	Boid* b = &mBoids[1];
	vec3f p = b->transform->GetPosition();
	char str[256];
	sprintf_s(str, "Group Motion Sample  S: %f  A: %f  C: %f", gSeparationWeight, gAlignmentWeight, gCohesionWeight);
	mRenderer->SetWindowCaption(str);
}

void GroupMotionSample::UpdateInput(Input& input)
{
	ScreenPoint mousePosition = Input::SharedInstance().mousePosition;
	if (input.GetMouseButton(MOUSEBUTTON_LEFT)) 
	{
		mCamera.mTransform.RotatePitch(-(mousePosition.y - mMouseY) * RADIAN * CAMERA_ROTATION_SPEED);
		mCamera.mTransform.RotateYaw(-(mousePosition.x - mMouseX) * RADIAN * CAMERA_ROTATION_SPEED);
	}

	mMouseX = mousePosition.x;
	mMouseY = mousePosition.y;

	vec3f position = mCamera.mTransform.GetPosition();
	if (input.GetKey(KEYCODE_W)) 
	{
		position += mCamera.mTransform.GetForward() * CAMERA_SPEED;
		mCamera.mTransform.SetPosition(position);
	}

	if (input.GetKey(KEYCODE_A)) 
	{
		position += mCamera.mTransform.GetRight() * -CAMERA_SPEED;
		mCamera.mTransform.SetPosition(position);
	}

	if (input.GetKey(KEYCODE_D)) 
	{
		position += mCamera.mTransform.GetRight() * CAMERA_SPEED;
		mCamera.mTransform.SetPosition(position);
	}

	if (input.GetKey(KEYCODE_S)) 
	{
		position += mCamera.mTransform.GetForward() * -CAMERA_SPEED;
		mCamera.mTransform.SetPosition(position);
	}

	if (input.GetKey(KEYCODE_R))
	{
		ResetBoids();
	}

	float step = 0.005f;
	if (input.GetKey(KEYCODE_C) && input.GetKeyDown(KEYCODE_UP))
	{
		gCohesionWeight += step;
	}

	if (input.GetKey(KEYCODE_C) && input.GetKeyDown(KEYCODE_DOWN))
	{
		gCohesionWeight -= step;
	}

	if (input.GetKey(KEYCODE_V) && input.GetKeyDown(KEYCODE_UP))
	{
		gAlignmentWeight += step;
	}

	if (input.GetKey(KEYCODE_V) && input.GetKeyDown(KEYCODE_DOWN))
	{
		gAlignmentWeight -= step;
	}

	if (input.GetKey(KEYCODE_O) && input.GetKeyDown(KEYCODE_UP))
	{
		gSeparationWeight += step;
	}

	if (input.GetKey(KEYCODE_O) && input.GetKeyDown(KEYCODE_DOWN))
	{
		gSeparationWeight -= step;
	}
}

void GroupMotionSample::UpdateBoidNeighbors()
{
	for (int i = 0; i < INSTANCE_COUNT; i++)
	{
		Boid* boid = &mBoids[i];
		for (int j = 0; j < INSTANCE_COUNT; j++)
		{
			if (i == j)
			{
				continue;
			}

			vec3f nPosition		= mBoids[j].transform->GetPosition();
			vec3f toNeighbor	= nPosition - boid->transform->GetPosition();
			float distance		= cliqCity::graphicsMath::magnitude(toNeighbor);

			if (distance < boid->nDistances[0])
			{
				boid->nPositions[2] = boid->nPositions[1];
				boid->nPositions[1] = boid->nPositions[0];
				boid->nPositions[0] = nPosition;

				boid->nDirections[2] = boid->nDirections[1];
				boid->nDirections[1] = boid->nDirections[0];
				boid->nDirections[0] = toNeighbor;

				boid->nDistances[2] = boid->nDistances[1];
				boid->nDistances[1] = boid->nDistances[0];
				boid->nDistances[0] = distance;

				boid->nIndices[2] = boid->nIndices[1];
				boid->nIndices[1] = boid->nIndices[0];
				boid->nIndices[0] = j;
			}
			else if (distance < boid->nDistances[1])
			{
				boid->nPositions[2] = boid->nPositions[1];
				boid->nPositions[1] = nPosition;

				boid->nDirections[2] = boid->nDirections[1];
				boid->nDirections[1] = toNeighbor;

				boid->nDistances[2] = boid->nDistances[1];
				boid->nDistances[1] = distance;

				boid->nIndices[2] = boid->nIndices[1];
				boid->nIndices[1] = j;
			}
			else if (distance < boid->nDistances[2])
			{
				boid->nPositions[2] = nPosition;

				boid->nDirections[2] = toNeighbor;

				boid->nDistances[2] = distance;

				boid->nIndices[2] = j;
			}
		}
	}
}

void GroupMotionSample::UpdateBoidBehaviors()
{
	for (int i = 0; i < INSTANCE_COUNT; i++)
	{
		Boid* boid = &mBoids[i];

		vec3f collisionAvoidance = 0.0f;
		vec3f averageVelocity = 0.0f;
		vec3f flockDirection = 0.0f;

		float cAccu = 0.0f;
		float oAccu = 0.0f;
		float vAccu = 0.0f;
		float fAccu = 0.0f;

		float collisionCount = 0;

		for (int j = 0; j < 3; j++)
		{
			if (IntersectSphereSphere(*boid->collider, *mBoids[boid->nIndices[j]].collider))
			{
				collisionAvoidance -= boid->nDirections[j];
				collisionCount++;
			}

			averageVelocity += mBoids[boid->nIndices[j]].rigidBody->velocity;
			flockDirection += boid->nPositions[j];
		}

		collisionAvoidance	/= collisionCount;
		averageVelocity		*= 0.33333333333f;
		flockDirection		*= 0.33333333333f;

		flockDirection = flockDirection - boid->transform->GetPosition();
	//	averageVelocity = averageVelocity - boid->rigidBody->velocity;

		collisionAvoidance	= cliqCity::graphicsMath::normalize(collisionAvoidance);
		averageVelocity		= cliqCity::graphicsMath::normalize(averageVelocity);
		flockDirection		= cliqCity::graphicsMath::normalize(flockDirection);

		collisionAvoidance	*= gSeparationWeight;
		averageVelocity		*= gAlignmentWeight;
		flockDirection		*= gCohesionWeight;

		boid->rigidBody->velocity = collisionAvoidance + averageVelocity + flockDirection;
	}
}

void GroupMotionSample::UpdateBoidForces()
{
	for (int i = 0; i < INSTANCE_COUNT; i++)
	{
		//Boid* boid = &mBoids[i];

		//vec3f obstacleAvoidance = 0.0f;
		//for (int j = 0; j < 6; j++)
		//{
		//	float d = cliqCity::graphicsMath::dot(mPlanes[j].normal, boid->collider->origin) - mPlanes[j].distance;
		//	obstacleAvoidance += (mPlanes[j].normal * (1.0f / d));
		//}

		//boid->rigidBody->forces += obstacleAvoidance * (1.0f / 6.0f);

		vec3f position = mBoids[i].transform->GetPosition();
		vec3f velocity = mBoids[i].rigidBody->velocity;

		float p = 5;
		if ((position.x < -p && velocity.x < 0.0f) || (position.x > p && velocity.x > 0.0f))
		{
			velocity.x = -velocity.x;
		}

		if ((position.y < -p && velocity.y < 0.0f) || (position.y > p && velocity.y > 0.0f))
		{
			velocity.y = -velocity.y;
		}

		if ((position.z < -p && velocity.z < 0.0f) || (position.z > p && velocity.z > 0.0f))
		{
			velocity.z = -velocity.z;
		}

		mBoids[i].rigidBody->velocity = velocity;
	}
}

void GroupMotionSample::IntegrateBoids(float deltaTime)
{
	if (deltaTime > 0.01667f)
	{
		deltaTime = 0.01667f;
	}

	static float accumulator = 0.0f;
	accumulator += deltaTime;

	while (accumulator >= PHYSICS_TIME_STEP)
	{
		static const vec3f zero = vec3f(0.0f);

		for (int i = 0; i < INSTANCE_COUNT; i++)
		{
			Boid* boid = &mBoids[i];
			vec3f velocity = boid->rigidBody->velocity;
			vec3f position = boid->transform->GetPosition();
			vec3f rotation = boid->transform->GetRollPitchYaw();

			position += velocity * deltaTime;
			rotation = { -PI * 0.5f, 0.0f, atan2(velocity.y, velocity.x) };

			boid->rigidBody->velocity = velocity;
			boid->transform->SetPosition(position);
			boid->transform->SetRotation(rotation);
			boid->collider->origin = position;
			boid->rigidBody->forces = zero;
		}

		accumulator -= PHYSICS_TIME_STEP;
	}


}

void GroupMotionSample::ResetBoids()
{
	float angle = 2.0f * PI / INSTANCE_COUNT;
	for (int i = 0; i < INSTANCE_COUNT; i++)
	{
		vec3f scale = { 1.0f, 1.0f, 1.0f };
		vec3f position = { cos(angle * i), sin(angle*  i), 0.0f };
		quatf rotation = { 1.0f, 0.0f, 0.0f, 0.0f };
		//vec3f velocity = { static_cast<float>(RAND(2, 0.2f)), static_cast<float>(RAND(3, 0.1f), 0.0f) };
		vec3f velocity = vec3f(5.0f);

		mBoidTransforms[i].SetPosition(position);
		mBoidTransforms[i].SetScale(scale);
		mBoidTransforms[i].SetRotation(rotation);

		mBoidRigidBodies[i].velocity = velocity;

		mBoidColliders[i].origin = position;
		mBoidColliders[i].radius = BOID_COLLIDER_RADIUS;

		mBoids[i].transform = &mBoidTransforms[i];
		mBoids[i].rigidBody = &mBoidRigidBodies[i];
		mBoids[i].collider = &mBoidColliders[i];

		mBoids[i].nDistances[0] = FLT_MAX;
		mBoids[i].nDistances[1] = FLT_MAX;
		mBoids[i].nDistances[2] = FLT_MAX;
	}
}

void GroupMotionSample::UpdateShaderResources()
{
	for (int i = 0; i < INSTANCE_COUNT; i++)
	{
		mBoidWorldMatrices[i] = mBoidTransforms[i].GetWorldMatrix().transpose();
	}

	mViewProjection.view = mCamera.GetViewMatrix().transpose();
	mViewProjection.projection = mCamera.GetProjectionMatrix().transpose();
}

void GroupMotionSample::VRender()
{
	float color[] = { 1.0f, 1.0f, 1.0f, 1.0f };

	mRenderer->VSetContextTargetWithDepth();
	mRenderer->VClearContext(color, 1.0f, 0);

	mRenderer->VSetPrimitiveType(GPU_PRIMITIVE_TYPE_TRIANGLE);
	mRenderer->VSetInputLayout(mBoidVertexShader);
	mRenderer->VSetVertexShader(mBoidVertexShader);
	mRenderer->VSetPixelShader(mBoidPixelShader);

	mRenderer->VBindMesh(mBoidMesh);

	mRenderer->VUpdateShaderInstanceBuffer(mBoidShaderResouce, mBoidWorldMatrices, sizeof(mat4f) * INSTANCE_COUNT, 0);
	mRenderer->VUpdateShaderConstantBuffer(mBoidShaderResouce, &mViewProjection, 0);
	mRenderer->VSetVertexShaderInstanceBuffers(mBoidShaderResouce);
	mRenderer->VSetVertexShaderConstantBuffers(mBoidShaderResouce);

	ID3D11DeviceContext* deviceContext = static_cast<DX3D11Renderer*>(mRenderer)->GetDeviceContext();
	deviceContext->DrawIndexedInstanced(mBoidMesh->GetIndexCount(), INSTANCE_COUNT, 0, 0, 0);

	mRenderer->VSwapBuffers();
}

void GroupMotionSample::VShutdown()
{
	mBoidVertexShader->~IShader();
	mBoidPixelShader->~IShader();
	mBoidShaderResouce->~IShaderResource();
	mBoidMesh->~IMesh();
	mLinearAllocator.Free();
}

void GroupMotionSample::VOnResize()
{
	mCamera.SetProjectionMatrix(mat4f::normalizedPerspectiveLH(PI * 0.25f, mRenderer->GetAspectRatio(), 0.1f, 100.0f));
}

DECLARE_MAIN(GroupMotionSample);