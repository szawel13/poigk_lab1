//Paweł Szałwiński 198073 ACIR 4A

#include <vector>
#include <algorithm>
#include <functional> 
#include <memory>
#include <cstdlib>
#include <cmath>
#include <ctime>

#include <raylib.h>
#include <raymath.h>

// --- UTILS ---
namespace Utils {
	inline static float RandomFloat(float min, float max) {
		return min + static_cast<float>(rand()) / RAND_MAX * (max - min);
	}
}

// --- TRANSFORM, PHYSICS, LIFETIME, RENDERABLE ---
struct TransformA {
	Vector2 position{};
	float rotation{};
};

struct Physics {
	Vector2 velocity{};
	float rotationSpeed{};
};

struct Renderable {
	enum Size { SMALL = 1, MEDIUM = 2, LARGE = 4 } size = SMALL;
};

// --- RENDERER ---
class Renderer {
public:
	static Renderer& Instance() {
		static Renderer inst;
		return inst;
	}

	void Init(int w, int h, const char* title) {
		InitWindow(w, h, title);
		SetTargetFPS(60);
		screenW = w;
		screenH = h;
	}

	void Begin() {
		BeginDrawing();
		ClearBackground(BLACK);
	}

	void End() {
		EndDrawing();
	}

	void DrawPoly(const Vector2& pos, int sides, float radius, float rot) {
		DrawPolyLines(pos, sides, radius, rot, WHITE);
	}

	int Width() const {
		return screenW;
	}

	int Height() const {
		return screenH;
	}

private:
	Renderer() = default;

	int screenW{};
	int screenH{};
};

// --- ASTEROID HIERARCHY ---

class Asteroid {
public:

	void SetPosition(const Vector2& pos) {
	transform.position = pos;
	}

	void TakeDamage(int dmg) {
	hp -= dmg;
	if (hp < 0) hp = 0;
	}

	float GetScale() const {
		return 0.15f * ((float)hp / maxHP);
	}

	Asteroid(int screenW, int screenH) {
		LoadTextureIfNeeded();
		init(screenW, screenH);
	}
	virtual ~Asteroid() = default;

	bool Update(float dt) {
		transform.position = Vector2Add(transform.position, Vector2Scale(physics.velocity, dt));
		transform.rotation += physics.rotationSpeed * dt;
		if (transform.position.x < -GetRadius() || transform.position.x > Renderer::Instance().Width() + GetRadius() ||
			transform.position.y < -GetRadius() || transform.position.y > Renderer::Instance().Height() + GetRadius())
			return false;
		return true;
	}
	virtual void Draw() const = 0;

	Vector2 GetPosition() const {
		return transform.position;
	}

	float constexpr GetRadius() const {
		return 16.f * (float)render.size;
	}

	int GetDamage() const {
		return baseDamage * static_cast<int>(render.size);
	}

	int GetSize() const {
		return static_cast<int>(render.size);
	}

	static Texture2D textureSmall;
	static bool textureSmallLoaded;
	static Texture2D texture;
	static bool textureLoaded;
	static void LoadTextureIfNeeded() {
		if (!textureLoaded) {
			texture = LoadTexture("textures/speedbumpsign.png");
			SetTextureFilter(texture, TEXTURE_FILTER_BILINEAR);
			GenTextureMipmaps(&texture);
			textureLoaded = true;
		}
		if (!textureSmallLoaded) {
			textureSmall = LoadTexture("textures/speedbump.png");
			SetTextureFilter(textureSmall, TEXTURE_FILTER_BILINEAR);
			GenTextureMipmaps(&textureSmall);
			textureSmallLoaded = true;
		}
	}
	int maxHP = 20;
	int hp = maxHP;
	bool isChild = false;

protected:
	
	void init(int screenW, int screenH) {
		// Choose size
		render.size = static_cast<Renderable::Size>(1 << GetRandomValue(0, 2));

		maxHP = 10 * static_cast<int>(render.size);
		hp = maxHP;

		// Spawn at random edge
		switch (GetRandomValue(0, 3)) {
		case 0:
			transform.position = { Utils::RandomFloat(0, screenW), -GetRadius() };
			break;
		case 1:
			transform.position = { screenW + GetRadius(), Utils::RandomFloat(0, screenH) };
			break;
		case 2:
			transform.position = { Utils::RandomFloat(0, screenW), screenH + GetRadius() };
			break;
		default:
			transform.position = { -GetRadius(), Utils::RandomFloat(0, screenH) };
			break;
		}

		// Aim towards center with jitter
		float maxOff = fminf(screenW, screenH) * 0.1f;
		float ang = Utils::RandomFloat(0, 2 * PI);
		float rad = Utils::RandomFloat(0, maxOff);
		Vector2 center = {
										 screenW * 0.5f + cosf(ang) * rad,
										 screenH * 0.5f + sinf(ang) * rad
		};

		Vector2 dir = Vector2Normalize(Vector2Subtract(center, transform.position));
		physics.velocity = Vector2Scale(dir, Utils::RandomFloat(SPEED_MIN, SPEED_MAX));
		physics.rotationSpeed = Utils::RandomFloat(ROT_MIN, ROT_MAX);

		transform.rotation = Utils::RandomFloat(0, 360);
	}

	TransformA transform;
	Physics    physics;
	Renderable render;

	int baseDamage = 0;
	static constexpr float LIFE = 10.f;
	static constexpr float SPEED_MIN = 125.f;
	static constexpr float SPEED_MAX = 250.f;
	static constexpr float ROT_MIN = 50.f;
	static constexpr float ROT_MAX = 240.f;
};

class TriangleAsteroid : public Asteroid {
public:
	TriangleAsteroid(int w, int h) : Asteroid(w, h) { baseDamage = 5; }
	void Draw() const override {
	float scale = GetScale();
	Vector2 dstPos = {
		transform.position.x - (texture.width * scale) * 0.5f,
		transform.position.y - (texture.height * scale) * 0.5f
	};
	DrawTextureEx(isChild ? textureSmall : texture, dstPos, transform.rotation, scale, WHITE);
	DrawText(TextFormat("hp: %d", hp), transform.position.x, transform.position.y - 30, 14, YELLOW);

		}
	};
	class SquareAsteroid : public Asteroid {
public:
	SquareAsteroid(int w, int h) : Asteroid(w, h) { baseDamage = 5; }
	void Draw() const override {
	float scale = GetScale();
	Vector2 dstPos = {
		transform.position.x - (texture.width * scale) * 0.5f,
		transform.position.y - (texture.height * scale) * 0.5f
	};
	DrawTextureEx(isChild ? textureSmall : texture, dstPos, transform.rotation, scale, WHITE);
	DrawText(TextFormat("hp: %d", hp), transform.position.x, transform.position.y - 30, 14, YELLOW);

	}
};
class PentagonAsteroid : public Asteroid {
public:
	PentagonAsteroid(int w, int h) : Asteroid(w, h) { baseDamage = 5; }
	void Draw() const override {
	float scale = GetScale();
	Vector2 dstPos = {
		transform.position.x - (texture.width * scale) * 0.5f,
		transform.position.y - (texture.height * scale) * 0.5f
	};
	DrawTextureEx(isChild ? textureSmall : texture, dstPos, transform.rotation, scale, WHITE);
	DrawText(TextFormat("hp: %d", hp), transform.position.x, transform.position.y - 30, 14, YELLOW);

	}
};

// Shape selector
enum class AsteroidShape { TRIANGLE = 3, SQUARE = 4, PENTAGON = 5, RANDOM = 0 };

// Factory
static inline std::unique_ptr<Asteroid> MakeAsteroid(int w, int h, AsteroidShape shape) {
	switch (shape) {
	case AsteroidShape::TRIANGLE:
		return std::make_unique<TriangleAsteroid>(w, h);
	case AsteroidShape::SQUARE:
		return std::make_unique<SquareAsteroid>(w, h);
	case AsteroidShape::PENTAGON:
		return std::make_unique<PentagonAsteroid>(w, h);
	default: {
		return MakeAsteroid(w, h, static_cast<AsteroidShape>(3 + GetRandomValue(0, 2)));
	}
	}
}

// --- PROJECTILE HIERARCHY ---
enum class WeaponType { LASER, BULLET, ROCKET, COUNT };
class Projectile {
public:

	Projectile(Vector2 pos, Vector2 vel, int dmg, WeaponType wt)
	{
		transform.position = pos;
		physics.velocity = vel;
		baseDamage = dmg;
		type = wt;
		if (!texturesLoaded) {
			texLaser = LoadTexture("textures/hammer.png");   
			texBullet = LoadTexture("textures/axe.png");     
			texRocket = LoadTexture("textures/papaj.png");   

			SetTextureFilter(texLaser, TEXTURE_FILTER_BILINEAR);
			SetTextureFilter(texBullet, TEXTURE_FILTER_BILINEAR);
			SetTextureFilter(texRocket, TEXTURE_FILTER_BILINEAR);

			GenTextureMipmaps(&texLaser);
			GenTextureMipmaps(&texBullet);
			GenTextureMipmaps(&texRocket);

			texturesLoaded = true;
		}
	}
	bool Update(float dt) {
		transform.position = Vector2Add(transform.position, Vector2Scale(physics.velocity, dt));

		if (transform.position.x < 0 ||
			transform.position.x > Renderer::Instance().Width() ||
			transform.position.y < 0 ||
			transform.position.y > Renderer::Instance().Height())
		{
			return true;
		}
		return false;
	}
	void Draw() const {
		Vector2 pos = transform.position;
		float scale = 0.05f;

		switch (type) {
		case WeaponType::LASER:
			DrawTextureEx(texLaser, { pos.x - texLaser.width * scale * 0.5f, pos.y - texLaser.height * scale * 0.5f }, 0, scale, WHITE);
			break;
		case WeaponType::BULLET:
			DrawTextureEx(texBullet, { pos.x - texBullet.width * scale * 0.5f, pos.y - texBullet.height * scale * 0.5f }, 0, scale, WHITE);
			break;
		case WeaponType::ROCKET:
			DrawTextureEx(texRocket, { pos.x - texRocket.width * scale * 0.5f, pos.y - texRocket.height * scale * 0.5f }, 0, scale, WHITE);
			break;
		default:
			DrawCircleV(pos, 5.f, WHITE);
		}
	}
	Vector2 GetPosition() const {
		return transform.position;
	}

	float GetRadius() const {
		return (type == WeaponType::BULLET) ? 5.f : 2.f;
	}

	int GetDamage() const {
		return baseDamage;
	}

private:
	static Texture2D texLaser;
	static Texture2D texBullet;
	static Texture2D texRocket;
	static bool texturesLoaded;
	TransformA transform;
	Physics    physics;
	int        baseDamage;
	WeaponType type;
};

inline static Projectile MakeProjectile(WeaponType wt, const Vector2 pos, float speed) {
	Vector2 vel{ 0, -speed };

	switch (wt) {
	case WeaponType::LASER:
		return Projectile(pos, vel, 10, wt);
	case WeaponType::BULLET:
		return Projectile(pos, vel, 5, wt);
	case WeaponType::ROCKET:
		return Projectile(pos, vel, 2, wt);
	default:
		return Projectile(pos, vel, 5, WeaponType::BULLET);
	}
}

// --- SHIP HIERARCHY ---
class Ship {
public:
	Ship(int screenW, int screenH) {
		transform.position = {
												 screenW * 0.5f,
												 screenH * 0.5f
		};
		maxHP = 200;
		hp = maxHP;
		speed = 250.f;
		alive = true;

		// per-weapon fire rate & spacing
		fireRateLaser = 18.f; // shots/sec
		fireRateBullet = 22.f;
		spacingLaser = 40.f; // px between lasers
		spacingBullet = 20.f;
	}
	virtual ~Ship() = default;
	virtual void Update(float dt) = 0;
	virtual void Draw() const = 0;
	
	int GetMaxHP() const {
	return maxHP;
	}

	void TakeDamage(int dmg) {
		if (!alive) return;
		hp -= dmg;
		if (hp <= 0) alive = false;
	}

	bool IsAlive() const {
		return alive;
	}

	Vector2 GetPosition() const {
		return transform.position;
	}

	virtual float GetRadius() const = 0;

	int GetHP() const {
		return hp;
	}

	float GetFireRate(WeaponType wt) const {
		return (wt == WeaponType::LASER) ? fireRateLaser : fireRateBullet;
	}

	float GetSpacing(WeaponType wt) const {
		return (wt == WeaponType::LASER) ? spacingLaser : spacingBullet;
	}

protected:
	int maxHP = 200;

	TransformA transform;
	int        hp;
	float      speed;
	bool       alive;
	float      fireRateLaser;
	float      fireRateBullet;
	float      spacingLaser;
	float      spacingBullet;
};

class PlayerShip :public Ship {
public:
	PlayerShip(int w, int h) : Ship(w, h) {
		texture = LoadTexture("textures/malyfiut.png");
		GenTextureMipmaps(&texture);                                                        // Generate GPU mipmaps for a texture
		SetTextureFilter(texture, 2);
		scale = 0.25f;
	}
	~PlayerShip() {
		UnloadTexture(texture);
	}

	void Update(float dt) override {
		if (alive) {
			if (IsKeyDown(KEY_W)) transform.position.y -= speed * dt;
			if (IsKeyDown(KEY_S)) transform.position.y += speed * dt;
			if (IsKeyDown(KEY_A)) transform.position.x -= speed * dt;
			if (IsKeyDown(KEY_D)) transform.position.x += speed * dt;
		}
		else {
			transform.position.y += speed * dt;
		}
	}

	void Draw() const override {
		if (!alive && fmodf(GetTime(), 0.4f) > 0.2f) return;
		Vector2 dstPos = {
										 transform.position.x - (texture.width * scale) * 0.5f,
										 transform.position.y - (texture.height * scale) * 0.5f
		};
		DrawTextureEx(texture, dstPos, 0.0f, scale, WHITE);
	}

	float GetRadius() const override {
		return (texture.width * scale) * 0.5f;
	}

private:
	Texture2D texture;
	float     scale;
};

// --- APPLICATION ---
class Application {
public:
	static Application& Instance() {
		static Application inst;
		return inst;
	}

	void Run() {
		srand(static_cast<unsigned>(time(nullptr)));
		Renderer::Instance().Init(C_WIDTH, C_HEIGHT, "static car simulator OOP");

		auto player = std::make_unique<PlayerShip>(C_WIDTH, C_HEIGHT);

		float spawnTimer = 0.f;
		float spawnInterval = Utils::RandomFloat(C_SPAWN_MIN, C_SPAWN_MAX);
		WeaponType currentWeapon = WeaponType::LASER;
		float shotTimer = 0.f;

		while (!WindowShouldClose()) {
			float dt = GetFrameTime();
			spawnTimer += dt;

			// Update player
			player->Update(dt);

			// Restart logic
			if (!player->IsAlive() && IsKeyPressed(KEY_R)) {
				player = std::make_unique<PlayerShip>(C_WIDTH, C_HEIGHT);
				asteroids.clear();
				projectiles.clear();
				spawnTimer = 0.f;
				spawnInterval = Utils::RandomFloat(C_SPAWN_MIN, C_SPAWN_MAX);
			}
			// Asteroid shape switch
			if (IsKeyPressed(KEY_ONE)) {
				currentShape = AsteroidShape::TRIANGLE;
			}
			if (IsKeyPressed(KEY_TWO)) {
				currentShape = AsteroidShape::SQUARE;
			}
			if (IsKeyPressed(KEY_THREE)) {
				currentShape = AsteroidShape::PENTAGON;
			}
			if (IsKeyPressed(KEY_FOUR)) {
				currentShape = AsteroidShape::RANDOM;
			}

			// Weapon switch
			if (IsKeyPressed(KEY_TAB)) {
				currentWeapon = static_cast<WeaponType>((static_cast<int>(currentWeapon) + 1) % static_cast<int>(WeaponType::COUNT));
			}

			// Shooting
			{
				if (player->IsAlive() && IsKeyDown(KEY_SPACE)) {
					shotTimer += dt;
					float interval = 1.f / player->GetFireRate(currentWeapon);
					float projSpeed = player->GetSpacing(currentWeapon) * player->GetFireRate(currentWeapon);

					while (shotTimer >= interval) {
						Vector2 p = player->GetPosition();
						p.y -= player->GetRadius();
						projectiles.push_back(MakeProjectile(currentWeapon, p, projSpeed));
						shotTimer -= interval;
					}
				}
				else {
					float maxInterval = 1.f / player->GetFireRate(currentWeapon);

					if (shotTimer > maxInterval) {
						shotTimer = fmodf(shotTimer, maxInterval);
					}
				}
			}

			// Spawn asteroids
			if (spawnTimer >= spawnInterval && asteroids.size() < MAX_AST) {
				asteroids.push_back(MakeAsteroid(C_WIDTH, C_HEIGHT, currentShape));
				spawnTimer = 0.f;
				spawnInterval = Utils::RandomFloat(C_SPAWN_MIN, C_SPAWN_MAX);
			}

			// Update projectiles - check if in boundries and move them forward
			{
				auto projectile_to_remove = std::remove_if(projectiles.begin(), projectiles.end(),
					[dt](auto& projectile) {
						return projectile.Update(dt);
					});
				projectiles.erase(projectile_to_remove, projectiles.end());
			}

			// Projectile-Asteroid collisions O(n^2)
			for (auto pit = projectiles.begin(); pit != projectiles.end();) {
				bool removed = false;

				for (auto ait = asteroids.begin(); ait != asteroids.end(); ++ait) {
					float dist = Vector2Distance((*pit).GetPosition(), (*ait)->GetPosition());
					if (dist < (*pit).GetRadius() + (*ait)->GetRadius()) {
						(*ait)->TakeDamage((*pit).GetDamage());
						if ((*ait)->GetScale() <= 0.05f) {
							
							Vector2 pos = (*ait)->GetPosition();
							int shape = (*ait)->GetSize();
							bool isChild = (*ait)->isChild;

							
							ait = asteroids.erase(ait);

							
							if (!isChild) {
								for (int i = 0; i < 2; ++i) {
									auto newAsteroid = MakeAsteroid(C_WIDTH, C_HEIGHT, static_cast<AsteroidShape>(shape));
									Vector2 offsetPos = {
										pos.x + Utils::RandomFloat(-20, 20),
										pos.y + Utils::RandomFloat(-20, 20)
									};
									newAsteroid->SetPosition(offsetPos);
									newAsteroid->isChild = true;
									newAsteroid->maxHP = 10;
									newAsteroid->hp = 10;
									asteroids.push_back(std::move(newAsteroid));
								}
							}
						} else {
							++ait;
						}
						pit = projectiles.erase(pit);
						removed = true;
						break;
					}
				}
				if (!removed) {
					++pit;
				}
			}

			// Asteroid-Ship collisions
			{
				auto remove_collision =
					[&player, dt](auto& asteroid_ptr_like) -> bool {
					if (player->IsAlive()) {
						float dist = Vector2Distance(player->GetPosition(), asteroid_ptr_like->GetPosition());

						if (dist < player->GetRadius() + asteroid_ptr_like->GetRadius()) {
							player->TakeDamage(asteroid_ptr_like->GetDamage());
							return true; // Mark asteroid for removal due to collision
						}
					}
					if (!asteroid_ptr_like->Update(dt)) {
						return true;
					}
					return false; // Keep the asteroid
					};
				auto asteroid_to_remove = std::remove_if(asteroids.begin(), asteroids.end(), remove_collision);
				asteroids.erase(asteroid_to_remove, asteroids.end());
			}

			// Render everything
			{
				Renderer::Instance().Begin();

				DrawText(TextFormat("HP: %d", player->GetHP()),
					10, 30, 20, GREEN);


					float hpPercent = (float)player->GetHP() / (float)player->GetMaxHP();
				int barWidth = 200;
				int barHeight = 20;
				int barX = 10;
				int barY = 10;

				DrawRectangle(barX, barY, barWidth, barHeight, DARKGRAY);
				Color hpColor = (hpPercent > 0.5f) ? GREEN : (hpPercent > 0.2f ? ORANGE : RED);
				DrawRectangle(barX, barY, (int)(barWidth * hpPercent), barHeight, hpColor);
				DrawRectangleLines(barX, barY, barWidth, barHeight, RAYWHITE);


				DrawText(TextFormat("", player->GetHP()), barX, barY + 25, 20, GREEN);


				const char* weaponName = "";
				switch (currentWeapon) {
				case WeaponType::LASER:
					weaponName = "MOTYWATOR KINETYCZNY";
					break;
				case WeaponType::BULLET:
					weaponName = "SIEKIERA";
					break;
				case WeaponType::ROCKET:
					weaponName = "PAPAJ";
					break;
				default:
					weaponName = "UNKNOWN";
					break;
				}
				DrawText(TextFormat("Weapon: %s", weaponName), barX, barY + 40, 20, BLUE);

				for (const auto& projPtr : projectiles) {
					projPtr.Draw();
				}
				for (const auto& astPtr : asteroids) {
					astPtr->Draw();
				}

				player->Draw();

				Renderer::Instance().End();
			}
		}
	}

private:
	Application()
	{
		asteroids.reserve(1000);
		projectiles.reserve(10'000);
	};

	std::vector<std::unique_ptr<Asteroid>> asteroids;
	std::vector<Projectile> projectiles;

	AsteroidShape currentShape = AsteroidShape::TRIANGLE;

	static constexpr int C_WIDTH = 2000;
	static constexpr int C_HEIGHT = 2000;
	static constexpr size_t MAX_AST = 150;
	static constexpr float C_SPAWN_MIN = 0.5f;
	static constexpr float C_SPAWN_MAX = 3.0f;

	static constexpr int C_MAX_ASTEROIDS = 1000;
	static constexpr int C_MAX_PROJECTILES = 10'000;
};

Texture2D Asteroid::texture;
bool Asteroid::textureLoaded = false;

Texture2D Asteroid::textureSmall;
bool Asteroid::textureSmallLoaded = false;

Texture2D Projectile::texLaser;
Texture2D Projectile::texBullet;
Texture2D Projectile::texRocket;
bool Projectile::texturesLoaded = false;

int main() {
	Application::Instance().Run();
	return 0;
}
