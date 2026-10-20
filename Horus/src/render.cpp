#include "render.h"

// Max brightness a single bounce (or SSS walk) may return - removes fireflies (slightly biased)
static const float maxIndirect = 10.0f;

// Caps a sample's brightest channel at maxValue, keeping its hue - removes fireflies (slightly biased)
static Vector3D<float> clampSample(Vector3D<float> c, float maxValue)
{
	float m = (c.x > c.y) ? c.x : c.y;
	m = (m > c.z) ? m : c.z;

	if (m > maxValue)
	{
		c = c * (maxValue / m);
	}

	return c;
}

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

// Reflects a ray at a hit point based on the surface normal and the reflection direction, and accumulates the resulting color contribution.
void Integrator::reflect(Vector3D<float> hp, Vector3D<float> normal, Vector3D<float> refDir, BVH& bvh, int nBounces, float refGain, Vector3D<float>& col)
{
	Ray newRay(hp + (normal * 0.001f), refDir);
	col += clampSample(rayPath(newRay, bvh, nBounces - 1), maxIndirect) * refGain;
}

// Subsurface scattering simulation using a random walk approach.
Vector3D<float> Integrator::subsurfaceWalk(Ray& ray, BVH& bvh, const Medium& medium, float ior, int nBounces)
{
	Vector3D<float> sigmaTV = medium.getSigmaT();

	float maxSigmaT = std::max(sigmaTV.x, std::max(sigmaTV.y, sigmaTV.z));

	if (maxSigmaT <= 0.0f)
	{
		return rayPath(ray, bvh, nBounces - 1);
	}

	int hero = static_cast<int>(unitRandom.Generate() * 3.0f);

	float sigmaTHero = sigmaTV[hero];

	//float albedo = medium.getAlbedo();
	Vector3D<float> sigmaS = medium.getSigmaS();
	float g = medium.getG();

	bool scattered = false;

	Vector3D<float> subsurfaceColor(1.0f, 1.0f, 1.0f);
	Vector3D<float> radiance(0.0f, 0.0f, 0.0f);

	Ray currentRay = ray;

	const int maxScatter = 256;

	for (int i = 0; i < maxScatter; ++i)
	{

		float rnd = unitRandom.Generate();
		float tFlight = -std::log(1.0f - rnd) / sigmaTHero;

		HitRecord boundaryRecord;

		GeometryObject* boundaryHit = bvh.traversal(currentRay, currentRay.getTMin(), currentRay.getTMax(), boundaryRecord);

		if (boundaryHit == nullptr)
		{
			return radiance + (subsurfaceColor % rayPath(currentRay, bvh, nBounces));
		}

		float tBoundary = boundaryRecord.t;

		if (tFlight >= tBoundary)
		{
			Vector3D<float> exitPoint = currentRay.getPointAt(tBoundary);
			Vector3D<float> exitDir = currentRay.getDirection();

			Vector3D<float> boundaryWeight;

			boundaryWeight.x = std::exp(-(sigmaTV.x - sigmaTHero) * tBoundary);
			boundaryWeight.y = std::exp(-(sigmaTV.y - sigmaTHero) * tBoundary);
			boundaryWeight.z = std::exp(-(sigmaTV.z - sigmaTHero) * tBoundary);

			subsurfaceColor = subsurfaceColor % boundaryWeight;

			Vector3D<float> normal = boundaryHit->computeNormal(boundaryRecord);
			
			Vector3D<float> dir = exitDir;
			dir.normalize();

			float d = dir * normal;
			float ratio;

			if (d < 0.0f)
			{
				ratio = 1.0f / ior;
			}
			else
			{
				ratio = ior / 1.0f;
				normal = -normal;
			}

			float cos_i = -(dir * normal);

			float k = 1.0f - (ratio * ratio) * (1.0f - (cos_i * cos_i));

			if (k < 0.0f)
			{
				currentRay.reflect(normal);
				Vector3D<float> reflectedDir = currentRay.getDirection();
				currentRay.setOrigin(exitPoint + (reflectedDir * 0.001f));

				continue;
			}

			//Ray exitRay(exitPoint + (exitDir * 0.001f), exitDir);

			//return radiance + (subsurfaceColor % rayPath(exitRay, bvh, nBounces - 1, false));

			float cos_schlick = (ratio > 1.0f) ? std::sqrt(k) : cos_i;

			float F_0 = ((1.0f - ior) / (1.0 + ior)) * ((1.0f - ior) / (1.0 + ior));
			float F = F_0 + (1.0f - F_0) * pow(1.0f - cos_schlick, 5.0f);

			if (std::abs(ior - 1.0f) < 1e-6f)
			{
				F = 0;
			}

			if (unitRandom.Generate() < F)
			{
				currentRay.reflect(normal);
				Vector3D<float> reflectedDir = currentRay.getDirection();
				currentRay.setOrigin(exitPoint + (reflectedDir * 0.001f));

				continue;
			}

			Ray exitRay(exitPoint - (normal * 0.001f), exitDir);
			exitRay.refract(exitDir, normal, k, cos_i, ratio);

			return radiance + (subsurfaceColor % rayPath(exitRay, bvh, nBounces - 1, !scattered));
		}

		Vector3D<float> scatterPoint = currentRay.getPointAt(tFlight);
		Vector3D<float> currentDir = currentRay.getDirection();

		scattered = true;

		Vector3D<float> channelWeight;

		channelWeight.x = (sigmaS.x / sigmaTHero) * std::exp(-(sigmaTV.x - sigmaTHero) * tFlight);
		channelWeight.y = (sigmaS.y / sigmaTHero) * std::exp(-(sigmaTV.y - sigmaTHero) * tFlight);
		channelWeight.z = (sigmaS.z / sigmaTHero) * std::exp(-(sigmaTV.z - sigmaTHero) * tFlight);

		AreaLight* areaLight = nullptr;
		MeshLight* meshLight = nullptr;

		Vector3D<float> lightPos;
		Vector3D<float> lightNormal;
		Vector3D<float> lightEmission;
		float pdfArea = 0.0f;
		size_t lightObjectCount = 0;

		if (sampleLightSource(areaLight, meshLight, lightPos, lightNormal, lightEmission, pdfArea, lightObjectCount))
		{
			Vector3D<float> lightDir = lightPos - scatterPoint;
			float distance = lightDir.getLength();
			lightDir.normalize();

			Ray toBoundary(scatterPoint, lightDir);

			HitRecord toBoundaryRecord;

			GeometryObject* boundaryHit = bvh.traversal(toBoundary, toBoundary.getTMin(), toBoundary.getTMax(), toBoundaryRecord);

			if (boundaryHit != nullptr)
			{
				float tBoundaryHit = toBoundaryRecord.t;
				Vector3D<float> pointBoundaryHit = toBoundary.getPointAt(tBoundaryHit);
				Vector3D<float> Tr;// = std::exp(-sigmaTHero * tBoundaryHit);

				Tr.x = std::exp(-sigmaTV.x * tBoundaryHit);
				Tr.y = std::exp(-sigmaTV.y * tBoundaryHit);
				Tr.z = std::exp(-sigmaTV.z * tBoundaryHit);

				float epsilon = 0.0001f;

				if (tBoundaryHit < distance)
				{
					Ray shadowRay(pointBoundaryHit + (lightDir * epsilon), lightDir);
					shadowRay.setTMax((distance - tBoundaryHit) - epsilon);

					HitRecord shadowHit;

					GeometryObject* occlusion = bvh.traversal(shadowRay, shadowRay.getTMin(), shadowRay.getTMax(), shadowHit);

					if (occlusion == nullptr || occlusion == areaLight || (meshLight != nullptr && occlusion->getMeshLight() == meshLight))
					{
						float cosLight = lightNormal * -lightDir;

						if (cosLight > 0.0f)
						{
							float cosTetha = currentDir * lightDir;
							float phase = Sampler::computePhase(g, cosTetha);

							float geometry = cosLight / (distance * distance);

							Vector3D<float> contribution = (subsurfaceColor % channelWeight % lightEmission % Tr) * (phase * geometry / pdfArea);

							radiance += contribution * lightObjectCount;
						}
					}
				}
			}
		}

		float r1 = unitRandom.Generate();
		float r2 = unitRandom.Generate();

		Vector3D<float> newDir = toWorld(Sampler::sampleHG(g, r1, r2), currentDir);
		newDir.normalize();

		currentRay.setOrigin(scatterPoint);
		currentRay.setDirection(newDir);

		subsurfaceColor = subsurfaceColor % channelWeight;

		if (i > 8)
		{
			float survival = std::max(subsurfaceColor.x, std::max(subsurfaceColor.y, subsurfaceColor.z));
			survival = std::min(survival, 0.95f);

			if (unitRandom.Generate() > survival)
			{
				return radiance;
			}

			subsurfaceColor *= (1.0f / survival);
		}
	}

	return radiance;
}

// Samples a light source (either an area light or a mesh light) and provides its position, normal, emission, and PDF for area sampling.
bool Integrator::sampleLightSource(AreaLight*& areaLight, MeshLight*& meshLight, Vector3D<float>& lightPos, Vector3D<float>& lightNormal, Vector3D<float>& lightEmission, float& pdfArea, size_t& lightObjectCount)
{
	areaLight = nullptr;
	meshLight = nullptr;
	
	lightObjectCount = areaLights.size() + meshLights.size();

	if (lightObjectCount == 0) { return false; }

	size_t rndLightIndex = static_cast<size_t>(unitRandom.Generate() * lightObjectCount);

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

	return true;
}

// Converts a light's area pdf into a solid-angle pdf seen from the shading point,
// including the 1/N chance sampleLightSource() had of picking this light.
float Integrator::lightPdfSolidAngle(float pdfArea, float distance, float cosLight)
{
	size_t lightCount = areaLights.size() + meshLights.size();

	if (cosLight <= 0.0f || lightCount == 0) { return 0.0f; }

	return (pdfArea * distance * distance) / (cosLight * static_cast<float>(lightCount));
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
Vector3D<float> Integrator::rayPath(Ray& ray, BVH& bvh, int nBounces, bool includeEmission, bool isPrimary, float bsdfPdf, Vector3D<float>* diffuseAOV, Vector3D<float>* specularAOV, Vector3D<float>* emissionAOV, Vector3D<float>* refractionAOV, Vector3D<float>* subsurfaceScatteringAOV, float* alpha, Vector3D<float>* normalAOV, Vector3D<float>* directLightAOV, Vector3D<float>* depthAOV, Vector3D<float>* positionAOV)
{
	HitRecord hit;

	GeometryObject* closestHit = bvh.traversal(ray, ray.getTMin(), ray.getTMax(), hit);
	float closestT = ray.getTMax();

	Vector3D<float> color(0.0f, 0.0f, 0.0f);

	if (closestHit)
	{
		// A camera sample that hits geometry counts towards the pixel's coverage (alpha)
		if (isPrimary && alpha)
		{
			*alpha += 1.0f;
		}

		if (isPrimary && depthAOV)
		{
			float dpv = hit.t * (ray.getDirection() * cameraForward);
			float nearest = std::min(depthAOV->x, dpv);
			*depthAOV = Vector3D<float>(nearest, 0.0f, 0.0f);
		}

		if (isPrimary && positionAOV)
		{
			Vector3D<float> distanceFromCameraStored = *positionAOV - ray.getOrigin();
			if ((hit.t * hit.t) * (ray.getDirection() * ray.getDirection()) < (distanceFromCameraStored * distanceFromCameraStored))
			{
				*positionAOV = ray.getPointAt(hit.t);
			}
		}

		auto& shader = closestHit->getShader();

		if (std::holds_alternative<Constant>(shader))
		{
			color = std::visit([](auto& p) { return p.getColor(); }, shader);

			return color;
		}
		else if (std::holds_alternative<Depth>(shader))
		{
			color = Vector3D<float>(1.0f / hit.t, 1.0f / hit.t, 1.0f / hit.t);

			return color;
		}

		if (nBounces == 0)
		{
			return color;
		}
		else
		{
			Vector3D<float> hitPoint = ray.getPointAt(hit.t);
			Vector3D<float> geometricNormal = closestHit->computeNormal(hit);
			Point<float> uv = closestHit->computeUV(hit);

			float diffuseGain = 1.0f;
			Vector3D<float> diffuseColor(1.0f, 1.0f, 1.0f);
			float roughness = 1.0f;
			float specular = 1.0f;

			float refraction_gain = 0.0f;
			float IOR = 1.0f;

			float subsurfaceGain = 0.0f;

			Vector3D<float> normal = geometricNormal;

			Medium medium;

			Vector3D<float> lightsContribution = Vector3D<float>(0.0f, 0.0f, 0.0f);

			if (auto surface = std::get_if<Surface>(&shader))
			{
				diffuseGain = surface->getDiffuseGain();

				surface->computeNormal(*closestHit, uv, normal);
				
				diffuseColor = surface->getDiffuseColor(uv);
				roughness = surface->getRoughness(uv);
				specular = surface->getSpecular(uv);

				refraction_gain = surface->getRefractionGain();
				IOR = surface->getIOR();

				subsurfaceGain = surface->getSubsurfaceGain(uv);

				// On a subsurface material the gain (or its map) is the weight between the surface (0) and the
				// subsurface walk (1). Without this, a weight of 0 would fall through to clear-glass refraction.
				if (surface->hasSubsurface())
				{
					refraction_gain = subsurfaceGain;
					subsurfaceGain = 1.0f;
				}

				medium = surface->getMedium();
			}

			if (isPrimary && normalAOV)
			{
				*normalAOV += normal;
			}

			Vector3D<float> viewDir = ray.getDirection();
			viewDir.normalize();

			float cosTheta = std::abs(viewDir * normal);

			float F = Schlick(IOR, cosTheta);

			if (std::abs(IOR - 1.0f) < 1e-4f)
			{
				F = 0.0f;
			}

			bool isDiffuseBranch = unitRandom.Generate() >= (F * specular);

			if (isDiffuseBranch && (!areaLights.empty() || !meshLights.empty()))
			{
				AreaLight* areaLight = nullptr;
				MeshLight* meshLight = nullptr;

				size_t lightObjectsCount = 0;

				Vector3D<float> lightPos = Vector3D<float>(0.0f, 0.0f, 0.0f);
				Vector3D<float> lightNormal = Vector3D<float>(0.0f, 0.0f, 0.0f);
				Vector3D<float> lightEmission = Vector3D<float>(0.0f, 0.0f, 0.0f);
				float pdfArea = 0.0f;

				Ray shadowRay = Ray();
				float epsilon = 0.001f;
				float distance = 0.0f;

				if (sampleLightSource(areaLight, meshLight, lightPos, lightNormal, lightEmission, pdfArea, lightObjectsCount))
				{
					Vector3D<float> shadowRayDir = lightPos - hitPoint;
					shadowRay.setOrigin(hitPoint + (geometricNormal * epsilon));

					distance = shadowRayDir.getLength();
					shadowRayDir.normalize();
					shadowRay.setDirection(shadowRayDir);
					shadowRay.setTMax(distance - epsilon);

					HitRecord shadowHit;

					GeometryObject* lightBVH = bvh.traversal(shadowRay, shadowRay.getTMin(), shadowRay.getTMax(), shadowHit);
					
					if (lightBVH == nullptr || lightBVH == areaLight || (meshLight != nullptr && lightBVH->getMeshLight() == meshLight))
					{
						float cos_surface = normal * shadowRay.getDirection();
						float cos_light = lightNormal * -shadowRay.getDirection();

						if (cos_surface >= 0 && cos_light >= 0)
						{
							float angleContribution = (cos_surface * cos_light) / (distance * distance);

							// MIS: the cosine-sampled diffuse bounce could have found this light too
							float lightPdf = lightPdfSolidAngle(pdfArea, distance, cos_light);
							float bsdfPdfLight = cos_surface / PI;
							float misWeight = PowerHeuristic(lightPdf, bsdfPdfLight);

							Vector3D<float> rddap = (lightEmission % diffuseColor) * diffuseGain * angleContribution / (pdfArea * PI) * (1.0f - refraction_gain) * misWeight;
							lightsContribution = rddap * lightObjectsCount;

							color += lightsContribution;
							
							if (diffuseAOV)
							{
								*diffuseAOV += lightsContribution;
							}

							if (directLightAOV)
							{
								*directLightAOV += lightsContribution;
							}
						}
					}
				}
			}

			// Dome light sampling
			for (LightObject* light : lights)
			{
				if (!isDiffuseBranch) { continue; }

				if (light->getLightType() != LightType::DOME) { continue; }

				float epsilon = 0.001f;

				DomeSample s = static_cast<DomeLightObject*>(light)->sampleDomeLight(unitRandom.Generate(), unitRandom.Generate());

				if (s.pdf <= 0.0f) { continue; }

				float cos_surface = normal * s.direction;

				if (cos_surface <= 0.0f) { continue; }

				Ray shadowRay(hitPoint + (geometricNormal * epsilon), s.direction);

				HitRecord shadowHit;

				Vector3D<float> dome_contribution = Vector3D<float>(0.0f, 0.0f, 0.0f);

				if (bvh.traversal(shadowRay, shadowRay.getTMin(), shadowRay.getTMax(), shadowHit) == nullptr)
				{
					//color += (s.radiance % diffuseColor) * diffuseGain * cos_surface / s.pdf;
					dome_contribution = (s.radiance % diffuseColor) * diffuseGain * cos_surface / (s.pdf * PI) * (1.0f - refraction_gain);
					color += dome_contribution;
				}

				if (diffuseAOV)
				{
					*diffuseAOV += dome_contribution;
				}

				if (directLightAOV)
				{
					*directLightAOV += dome_contribution;
				}
			}

			// Lights count in full for camera/mirror/glass rays, and MIS-weighted for GGX and diffuse rays (bsdfPdf > 0)
			if (includeEmission || bsdfPdf > 0.0f)
			{
				// distance travelled by the ray that found this light (ray direction may not be unit length)
				float lightDistance = (hitPoint - ray.getOrigin()).getLength();

				if (AreaLight* al = dynamic_cast<AreaLight*>(closestHit))
				{
					// MIS: if a GGX or diffuse ray found this light, light sampling could have found it too
					float misWeight = 1.0f;

					if (bsdfPdf > 0.0f)
					{
						float cosLight = al->getNormal() * -viewDir;
						float lightPdf = lightPdfSolidAngle(1.0f / (al->getWidth() * al->getHeight()), lightDistance, cosLight);

						// Back of the light: light sampling gives it nothing, so the MIS partner must too
						misWeight = (lightPdf > 0.0f) ? PowerHeuristic(bsdfPdf, lightPdf) : 0.0f;
					}

					Vector3D<float> emission = al->getColor() * al->getIntensity() * std::pow(2.0f, al->getExposure()) * misWeight;
					color += emission;

					if (emissionAOV)
					{
						*emissionAOV += emission;
					}
				}

				if (closestHit->getMeshLight() != nullptr)
				{
					MeshLight* ml = closestHit->getMeshLight();
					float misWeight = 1.0f;

					if (bsdfPdf > 0.0f)
					{
						float cosLight = geometricNormal * -viewDir;
						float lightPdf = lightPdfSolidAngle(ml->getPdfArea(), lightDistance, cosLight);

						// Back of the light: light sampling gives it nothing, so the MIS partner must too
						misWeight = (lightPdf > 0.0f) ? PowerHeuristic(bsdfPdf, lightPdf) : 0.0f;
					}

					//color += ml->getColor() * ml->getIntensity() * std::pow(2.0f, ml->getExposure()) * misWeight;
					Vector3D<float> meshLightContribution = ml->getColor() * ml->getIntensity() * std::pow(2.0f, ml->getExposure()) * misWeight;
					color += meshLightContribution;

					if (emissionAOV)
					{
						*emissionAOV += meshLightContribution;
					}
				}
			}

			// create new ray from hit point

			Ray reflected = ray;
			//Vector3D<float> normal = closestHit->getNormal();
			reflected.reflect(normal);
			Vector3D<float> reflectedDir = reflected.getDirection();

			Vector3D<float> shadingNormal = normal;

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
				
				float k = 1.0f - ratio * ratio * (1.0f - cos_i * cos_i);

				if (k < 0.0f)
				{
					// reflect()
					reflect(hitPoint, normal, reflectedDir, bvh, nBounces, refraction_gain, color);
				}
				else
				{
					// Step 2: Fresnel via Schlick
					float cos_schlick = (ratio > 1.0f) ? std::sqrt(k) : cos_i;

					//float F_0 = ((1.0f - IOR) / (1.0f + IOR)) * ((1.0f - IOR) / (1.0f + IOR));
					//float F = F_0 + (1.0f - F_0) * pow(1.0f - cos_schlick, 5.0f);
					float F = Schlick(IOR, cos_schlick);

					if (std::abs(IOR - 1.0f) < 1e-4f)
					{
						F = 0.0f;
					}

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

						Vector3D<float> refractionContribution(0.0f, 0.0f, 0.0f);
						
						if (subsurfaceGain > 0.0f)
						{
							refractionContribution = clampSample(subsurfaceWalk(refracted, bvh, medium, IOR, nBounces), maxIndirect) * refraction_gain * subsurfaceGain;
							color += refractionContribution;

							if (subsurfaceScatteringAOV)
							{
								*subsurfaceScatteringAOV += refractionContribution;
							}
							
						}
						else
						{
							refractionContribution = clampSample(rayPath(refracted, bvh, nBounces - 1), maxIndirect) * refraction_gain;
							color += refractionContribution;

							if (refractionAOV)
							{
								*refractionAOV += refractionContribution;
							}
						}
					}
				}
			}

			if (refraction_gain < 1.0f)
			{
				Vector3D<float> newDir;
				Vector3D<float> tint;
				bool passEmission;
				float bsdfPdf = 0.0f;
				Vector3D<float> specularDirect(0.0f, 0.0f, 0.0f);

				if (isDiffuseBranch)
				{
					Vector3D<float> rndDir = Sampler::cosineWeightSampleHemisphere(r1, r2);
					Vector3D<float> surfaceNormal = normal;

					newDir = toWorld(rndDir, surfaceNormal);
					tint = diffuseColor;
					passEmission = false;			// the dome stays with dome NEE
					bsdfPdf = rndDir.z / PI;		// cosine pdf: lets a light hit be MIS-weighted against light sampling
				}
				else
				{
					float alpha = roughness * roughness;

					Vector3D<float> wo = -viewDir;
					Vector3D<float> woLocal = toLocal(wo, shadingNormal);

					passEmission = true;

					if (woLocal.z <= 1e-6f)
					{
						newDir = reflectedDir;
						tint = Vector3D<float>(1.0f, 1.0f, 1.0f);
					}
					else
					{
						// --- Light sampling for the specular lobe (MIS partner of the VNDF sample) ---
						// Skipped for a near-mirror: a delta lobe can't be hit by a light sample.
						if (alpha >= 1e-3f && (!areaLights.empty() || !meshLights.empty()))
						{
							AreaLight* areaLight = nullptr;
							MeshLight* meshLight = nullptr;
							size_t lightCount = 0;

							Vector3D<float> lightPos(0.0f, 0.0f, 0.0f);
							Vector3D<float> lightNormal(0.0f, 0.0f, 0.0f);
							Vector3D<float> lightEmission(0.0f, 0.0f, 0.0f);
							float pdfArea = 0.0f;

							if (sampleLightSource(areaLight, meshLight, lightPos, lightNormal, lightEmission, pdfArea, lightCount))
							{
								Vector3D<float> toLight = lightPos - hitPoint;
								float distance = toLight.getLength();
								toLight.normalize();

								Vector3D<float> wiLight = toLocal(toLight, shadingNormal);
								float cosLight = lightNormal * -toLight;

								Vector3D<float> halfSum = woLocal + wiLight;
								float halfLen = halfSum.getLength();

								if (wiLight.z > 0.0f && cosLight > 0.0f && halfLen > 1e-4f)
								{
									Ray shadowRay(hitPoint + (geometricNormal * epsilon), toLight);
									shadowRay.setTMax(distance - epsilon);

									HitRecord shadowHit;
									GeometryObject* blocker = bvh.traversal(shadowRay, shadowRay.getTMin(), shadowRay.getTMax(), shadowHit);

									if (blocker == nullptr || blocker == areaLight || (meshLight != nullptr && blocker->getMeshLight() == meshLight))
									{
										Vector3D<float> wmLight = halfSum / halfLen;

										// f * cos(wi) without F*specular, which the branch choice already paid for
										float fCos = (GGXD(wmLight.z, alpha) * SmithG2(woLocal.z, wiLight.z, alpha)) / (4.0f * woLocal.z);

										float lightPdf = lightPdfSolidAngle(pdfArea, distance, cosLight);
										float bsdfPdfLight = GGXReflectionPdf(woLocal.z, wmLight.z, alpha);

										specularDirect = lightEmission * (fCos * PowerHeuristic(lightPdf, bsdfPdfLight) / lightPdf);
									}

									if (directLightAOV)
									{
										*directLightAOV += specularDirect * diffuseGain * (1.0f - refraction_gain);
									}
								}
							}
						}

						Vector3D<float> wh = Sampler::GGXVNDF(woLocal, alpha, r1, r2);
						Vector3D<float> wiLocal = (wh * 2.0f * (woLocal * wh)) - woLocal;

						if (wiLocal.z <= 0.0f)
						{
							newDir = reflectedDir;
							tint = Vector3D<float>(0.0f, 0.0f, 0.0f);
						}
						else
						{
							newDir = toWorld(wiLocal, shadingNormal);
							tint = Vector3D<float>(1.0f, 1.0f, 1.0f) * SmithG2OverG1(woLocal.z, wiLocal.z, alpha);

							if (alpha >= 1e-3f)
							{
								bsdfPdf = GGXReflectionPdf(woLocal.z, wh.z, alpha);
							}
						}
					}
				}

				Ray newRay(hitPoint + (geometricNormal * epsilon), newDir);

				Vector3D<float> indirectContribution = ((tint % clampSample(rayPath(newRay, bvh, nBounces - 1, passEmission, false, bsdfPdf), maxIndirect)) + specularDirect) * diffuseGain * (1.0f - refraction_gain);
				color += indirectContribution;

				if (diffuseAOV && isDiffuseBranch)
				{
					*diffuseAOV += indirectContribution;
				}

				if (specularAOV && !isDiffuseBranch)
				{
					*specularAOV += indirectContribution;
				}
			}
		}
	}
	else
	{
		if (includeEmission)
		{
			//background color
			bool domeVisible = false;

			for (LightObject* light : lights)
			{
				if (light->getLightType() == LightType::DOME && (light->getVisibility() || !isPrimary))
				{
					color += light->getRadiance(ray.getDirection());

					domeVisible = true;
				}
			}

			// A camera sample that sees a visible dome is covered too: the sky is part of the image
			if (isPrimary && alpha && domeVisible)
			{
				*alpha += 1.0f;
			}
		}
	}

	return color;
}

Vector3D<float> Integrator::toLocal(Vector3D<float> vec, Vector3D<float> refVector)
{
	Vector3D<float> refAxis(0.0f, 1.0f, 0.0f);

	if (fabs(refVector.y) > 0.999f)
	{
		refAxis = Vector3D<float>(1.0f, 0.0f, 0.0f);
	}

	Vector3D<float> T = refVector | refAxis;
	T.normalize();

	Vector3D<float> B = T | refVector;
	B.normalize();

	return Vector3D<float>(vec * T, vec * B, vec * refVector);
}