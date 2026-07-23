#include "render.h"

// Converts a vector from local space to world space based on the normal at the hit point and a reference vector.
Vector3D<float> Integrator::toWorld(Vector3D<float> v, Vector3D<float> refVector)
{
	// calculate one of the axis from the hitpoint by doing T = normal cross product reference axis
	Vector3D<float> referencceAxis(0.0f, 1.0f, 0.0f);

	if (fabs(refVector.y) > 0.999f)
	{
		referencceAxis = Vector3D<float>(1.0f, 0.0f, 0.0f);
	}

	Vector3D<float> T = refVector | referencceAxis;
	T.normalize();

	// do the cross product between T and the normal to get the other axis
	Vector3D<float> B = T | refVector;
	B.normalize();

	// convert the data
	Vector3D<float> worldVec = ((T * v.x) + (B * v.y) + (refVector * v.z));
	return worldVec;
}

void Integrator::reflect(Vector3D<float> hp, Vector3D<float> normal, Vector3D<float> refDir, BVH& bvh, int nBounces, float refGain, Vector3D<float>& col)
{
	Ray newRay(hp + (normal * 0.001f), refDir);
	col += rayPath(newRay, bvh, nBounces - 1) * refGain;
}

void Integrator::addAreaLights(std::vector<AreaLight*>& al)
{
	areaLights = al;
}

void Integrator::addMeshLights(std::vector<MeshLight*>& ml)
{
	meshLights = ml;
}

// Traces the path of a ray through the scene, calculating the color contribution at each intersection point.
Vector3D<float> Integrator::rayPath(Ray& ray, BVH& bvh, int nBounces)
{
	GeometryObject* closestHit = bvh.traversal(ray, ray.getTMin(), ray.getTMax());
	float closestT = ray.getTMax();

	Vector3D<float> color(0.0f, 0.0f, 0.0f);

	if (closestHit)
	{
		auto& shader = closestHit->getShader();

		if (std::holds_alternative<Constant>(shader))
		{
			color = std::visit([](auto& p) { return p.getColor(); }, shader);

			return color;
		}
		else if (std::holds_alternative<Depth>(shader))
		{
			color = Vector3D<float>(1.0f / closestHit->hitRecord.t, 1.0f / closestHit->hitRecord.t, 1.0f / closestHit->hitRecord.t);

			return color;
		}

		if (nBounces == 0)
		{
			return color;
		}
		else
		{
			Vector3D<float> hitPoint = ray.getPointat(closestHit->hitRecord.t);
			closestHit->computeNormal();

			float diffuseGain = 1.0f;
			Vector3D<float> diffuseColor(1.0f, 1.0f, 1.0f);
			float roughness = 1.0f;

			float refraction_gain = 0.0f;
			float IOR = 1.0f;

			Vector3D<float> lightsContribution = Vector3D<float>(0.0f, 0.0f, 0.0f);
			//Vector3D<float> meshLightsContribution = Vector3D<float>(0.0f, 0.0f, 0.0f);

			if (auto surface = std::get_if<Surface>(&shader))
			{
				diffuseGain = surface->getDiffuseGain();
				diffuseColor = surface->getDiffuseColor();
				roughness = surface->getRoughness();

				refraction_gain = surface->getRefractionGain();
				IOR = surface->getIOR();
			}

			// Area and meshLight light sampling
			// if (!areaLights.empty() || !meshLights.empty())
			if (!areaLights.empty() || !meshLights.empty())
			{
				//float rnd = unitRandom.Generate() * areaLights.size();
				AreaLight* areaLight = nullptr;
				MeshLight* meshLight = nullptr;

				float x_rnd = 0.0f;
				float z_rnd = 0.0f;

				size_t lightObjectsCount = areaLights.size() + meshLights.size();
				size_t rndLightIndex = static_cast<size_t>(unitRandom.Generate() * lightObjectsCount);

				Vector3D<float> lightPos = Vector3D<float>(0.0f, 0.0f, 0.0f);
				Vector3D<float> lightNormal = Vector3D<float>(0.0f, 0.0f, 0.0f);
				Vector3D<float> lightEmission = Vector3D<float>(0.0f, 0.0f, 0.0f);
				float pdfArea = 0.0f;

				Ray shadowRay = Ray();
				float epsilon = 0.001f;
				float distance = 0.0f;

				if (lightObjectsCount > 0)
				{
					if (rndLightIndex < areaLights.size())
					{
						// Handle area light
						areaLight = areaLights[static_cast<int>(rndLightIndex)];

						float x_rnd = (unitRandom.Generate() * areaLight->getWidth()) - areaLight->getWidth() * 0.5;
						float z_rnd = (unitRandom.Generate() * areaLight->getHeight()) - areaLight->getHeight() * 0.5;

						lightPos = areaLight->position + (areaLight->getRotationMatrix() * (Vector3D<float>(x_rnd, 0.0f, z_rnd)));
						lightNormal = areaLight->getNormal();
						lightEmission = areaLight->getColor() * areaLight->getIntensity() * std::pow(2.0f, areaLight->getExposure());
						pdfArea = 1.0f / (areaLight->getWidth() * areaLight->getHeight());
					}
					else
					{
						// Handle mesh light
						meshLight = meshLights[static_cast<int>(rndLightIndex - areaLights.size())];
						
						float rndTriangle = unitRandom.Generate();
						float rU = unitRandom.Generate();
						float rV = unitRandom.Generate();

						LightSample lightSample = meshLight->sampleLight(rndTriangle, rU, rV);
						lightPos = lightSample.position;
						lightNormal = lightSample.normal;
						lightEmission = meshLight->getColor() * meshLight->getIntensity() * std::pow(2.0f, meshLight->getExposure());
						pdfArea = lightSample.pdf;
					}
				}

				//Ray shadowRay = Ray();
				//float epsilon = 0.001f;
				//float distance = 0.0f;
				// drop 95 and 96 and loop through areaLights and meshLights, then pick one randomly. If it is an areaLight take line 96 and the do form 101 tp 108. If it is a meshLight do something else.
				//float x_rnd = (unitRandom.Generate() * areaLight->getWidth()) - areaLight->getWidth() * 0.5;
				//float z_rnd = (unitRandom.Generate() * areaLight->getHeight()) - areaLight->getHeight() * 0.5;

				//Vector3D<float> areaLightPos = areaLight->position + (areaLight->getRotationMatrix() * (Vector3D<float>(x_rnd, 0.0f, z_rnd)));

				Vector3D<float> shadowRayDir = lightPos - hitPoint;
				Vector3D<float> surfaceNormal = closestHit->getNormal();
				shadowRay.setOrigin(hitPoint + (surfaceNormal * epsilon));

				distance = shadowRayDir.getLength();
				shadowRayDir.normalize();
				shadowRay.setDirection(shadowRayDir);
				shadowRay.setTMax(distance - epsilon);

				GeometryObject* lightBVH = bvh.traversal(shadowRay, shadowRay.getTMin(), shadowRay.getTMax());

				if (lightBVH == nullptr || lightBVH == areaLight || (meshLight != nullptr && lightBVH->getMeshLight() == meshLight))
				{
					float cos_surface = surfaceNormal * shadowRay.getDirection();
					float cos_light = lightNormal * -shadowRay.getDirection();

					if (cos_surface >= 0 && cos_light >= 0)
					{
						float angleContribution = (cos_surface * cos_light) / (distance * distance);
						//Vector3D<float> rawLight = areaLight->getColor() * areaLight->getIntensity() * std::pow(2.0f, areaLight->getExposure());
						//float pdfArea = 1.0f / (areaLight->getWidth() * areaLight->getHeight());

						Vector3D<float> rddap = (lightEmission % diffuseColor) * diffuseGain * angleContribution / pdfArea;
						lightsContribution = rddap * lightObjectsCount;

						color += lightsContribution;
					}
				}
			}

			if (AreaLight* al = dynamic_cast<AreaLight*>(closestHit))
			{
				//AreaLight* areaLight = dynamic_cast<AreaLight*>(closestHit);
				color += al->getColor() * al->getIntensity() * std::pow(2.0f, al->getExposure());
			}

			if (closestHit->getMeshLight() != nullptr)
			{
				MeshLight* ml = closestHit->getMeshLight();
				color += ml->getColor() * ml->getIntensity() * std::pow(2.0f, ml->getExposure());
			}

			// create new ray from hit point

			Ray reflected = ray;
			Vector3D<float> normal = closestHit->getNormal();
			reflected.reflect(normal);
			Vector3D<float> reflectedDir = reflected.getDirection();

			float r1 = unitRandom.Generate();
			float r2 = unitRandom.Generate();

			float epsilon = 0.001f;

			// if refractionGain > 0.0f

			if (refraction_gain > 0.0f)
			{
			//	Refraction
			//	Determine which side the ray is on and set up accordingly
				Vector3D<float> dir = ray.getDirection(); // hitPoint - ray.getOrigin();
				dir.normalize();
				float d = dir * normal;

				float ratio;
			
				if (d < 0.0f) //if d · n̂ < 0
				{
					ratio = 1.0f / IOR;
					//	n̂ stays as - is
				}
				else
				{
					ratio = IOR / 1.0f;
					normal = -normal; //	n̂ = −n̂
				}
				// Step 1: Check for total internal reflection
				float cos_i = - (dir * normal);
				
				float k = 1.0f - ratio * ratio * (1.0f - cos_i * cos_i); //	k = 1 − η² ·(1 − cos_i²)

				if (k < 0.0f)//	if k < 0
				{
					// reflect()
					reflect(hitPoint, normal, reflectedDir, bvh, nBounces, refraction_gain, color);
				}
				else
				{
					// Step 2: Fresnel via Schlick
					float cos_schlick = (ratio > 1.0f) ? std::sqrt(k) : cos_i;

					float F_0 = ((1.0f - IOR) / (1.0f + IOR)) * ((1.0f - IOR) / (1.0f + IOR)); // F₀ = ((n₁ − n₂) / (n₁ + n₂))²
					float F = F_0 + (1.0f - F_0) * pow(1.0f - cos_schlick, 5.0f); // F = F₀ + (1 − F₀)(1 − cos_i)⁵

					//	 Step 3: Stochastic branch
					if ( unitRandom.Generate() < F) // if unitRandom.Generate() < F
					{
						// reflect()
						//Ray newRay(hitPoint + (normal * 0.001f), reflectedDir);
						//color += rayPath(newRay, bvh, nBounces - 1) * refraction_gain;
						reflect(hitPoint, normal, reflectedDir, bvh, nBounces, refraction_gain, color);
					}
					else
					{
					     //	refract()
						Ray refracted = ray;
						refracted.setOrigin(hitPoint - (normal * epsilon));
						refracted.refract(dir, normal, k, cos_i, ratio);
						color += rayPath(refracted, bvh, nBounces - 1) * refraction_gain;
					}
				}
			}
			if (refraction_gain < 1.0f)
			{
				Vector3D<float> rndDir = Sampler::cosineWeightSampleHemisphere(r1, r2);
				Vector3D<float> surfaceNormal = closestHit->getNormal();
				Vector3D<float> diffuseScatter = toWorld(rndDir, surfaceNormal);
				Vector3D<float> finalScatter = (reflectedDir * (1.0f - roughness)) + (diffuseScatter * roughness);

				Ray newRay(hitPoint + (surfaceNormal * epsilon), finalScatter);

				color += (diffuseColor % rayPath(newRay, bvh, nBounces - 1)) * diffuseGain * (1.0f - refraction_gain);
			}
		}
	}
	else
	{
		//background color
		for (LightObject* light : lights)
		{
			if (light->getLightType() == LightType::DOME)
			{
				color += light->getColor() * light->getIntensity();
			}
		}
	}

	return color;
}