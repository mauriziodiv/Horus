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

			if (auto surface = std::get_if<Surface>(&shader))
			{
				diffuseGain = surface->getDiffuseGain();
				diffuseColor = surface->getDiffuseColor();
				roughness = surface->getRoughness();

				refraction_gain = surface->getRefractionGain();
				IOR = surface->getIOR();
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
			else
			{
				Vector3D<float> rndDir = Sampler::cosineWeightSampleHemisphere(r1, r2);
				Vector3D<float> diffuseScatter = toWorld(rndDir, closestHit->getNormal());
				Vector3D<float> finalScatter = (reflectedDir * (1.0f - roughness)) + (diffuseScatter * roughness);

				Ray newRay(hitPoint + (normal * epsilon), finalScatter);

				color += (diffuseColor % rayPath(newRay, bvh, nBounces - 1)) * diffuseGain;
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