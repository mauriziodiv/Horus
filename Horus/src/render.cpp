#include "render.h"

Vector3D<float> Integrator::toWorld(Vector3D<float> v, Vector3D<float> normal)
{
	// calculate one of the axis from the hitpoint by doing T = normal cross product reference axis
	Vector3D<float> referencceAxis(0.0f, 1.0f, 0.0f);

	if (fabs(normal.y) > 0.999f)
	{
		referencceAxis = Vector3D<float>(1.0f, 0.0f, 0.0f);
	}

	Vector3D<float> T = normal | referencceAxis;
	T.normalize();

	// do the cross product between T and the normal to get the other axis
	Vector3D<float> B = T | normal;
	B.normalize();

	// convert the data
	Vector3D<float> worldVec = ((T * v.x) + (B * v.y) + (normal * v.z));
	return worldVec;
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

			if (auto surface = std::get_if<Surface>(&shader))
			{
				diffuseGain = surface->getDiffuseGain();
				diffuseColor = surface->getDiffuseColor();
				roughness = surface->getRoughness();
			}

			// create new ray from hit point
			float r1 = unitRandom.Generate();
			float r2 = unitRandom.Generate();
			Vector3D<float> rndDir = Sampler::cosineWeightSampleHemisphere(r1, r2);

			Vector3D<float> worldDir = toWorld(rndDir, closestHit->getNormal());
			
			Ray newRay(hitPoint, worldDir);

			color += (diffuseColor % rayPath(newRay, bvh, nBounces - 1)) * diffuseGain;
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