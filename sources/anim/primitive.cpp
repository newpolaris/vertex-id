#include "primitive.h"

std::vector<PrimitiveVertex> PlaneMesh(float size, float res, float UVScale) {

    const float SIZE = size;
    const int RES = static_cast<int>(res);

    size_t m_count = 3 * 2 * (RES * RES);

    std::vector<glm::vec3> positions;
    std::vector<glm::vec3> normals;
    std::vector<glm::vec2> texcoords;

    positions.resize(m_count);
    normals.resize(m_count, glm::vec3(0.0f, 1.0f, 0.0f));
    texcoords.resize(m_count);

    auto pPos = (glm::vec3*)positions.data();
    auto pUV = (glm::vec2*)texcoords.data();

    const float Delta = 1.0f / float(RES);

    for (int j = 0; j < RES; ++j)
    {
        for (int i = 0; i < RES; ++i)
        {
            glm::vec4 vInd = Delta * glm::vec4(i, j, i + 1, j + 1);

            *pUV = glm::vec2(vInd.x, vInd.y);
            *pPos = SIZE * glm::vec3(pUV->x - 0.5f, 0.0f, pUV->y - 0.5f);
            *pUV *= UVScale;
            ++pPos; ++pUV;

            *pUV = glm::vec2(vInd.x, vInd.w);
            *pPos = SIZE * glm::vec3(pUV->x - 0.5f, 0.0f, pUV->y - 0.5f);
            *pUV *= UVScale;
            ++pPos; ++pUV;

            *pUV = glm::vec2(vInd.z, vInd.y);
            *pPos = SIZE * glm::vec3(pUV->x - 0.5f, 0.0f, pUV->y - 0.5f);
            *pUV *= UVScale;
            ++pPos; ++pUV;


            *pUV = pUV[-1];
            *pPos = pPos[-1];
            ++pPos; ++pUV;

            *pUV = pUV[-3];
            *pPos = pPos[-3];
            ++pPos; ++pUV;

            *pUV = glm::vec2(vInd.z, vInd.w);
            *pPos = SIZE * glm::vec3(pUV->x - 0.5f, 0.0f, pUV->y - 0.5f);
            *pUV *= UVScale;
            ++pPos; ++pUV;
        }
    }

    for (auto& coord : texcoords)
        coord.g = 1 - coord.g;

    std::vector<PrimitiveVertex> data(m_count);
    for (size_t i = 0; i < m_count; i++) {
        data[i].position = positions[i];
        data[i].normal = normals[i];
        data[i].texcoord = texcoords[i];
    }
    return data;
}

std::vector<PrimitiveVertex> SphereMesh(float radius, size_t meshResolution) {
    size_t m_count = 2 * meshResolution * (meshResolution + 2);

    std::vector<glm::vec3> positions;
    std::vector<glm::vec3> normals;
    std::vector<glm::vec2> texcoords;

    positions.resize(m_count);
    normals.resize(m_count);
    texcoords.resize(m_count);

    glm::vec3* pPos = &(positions[0]);
    glm::vec3* pNor = &(normals[0]);
    glm::vec2* pUV = &(texcoords[0]);

    float theta2, phi;    // next theta angle, phi angle
    float ct, st;         // cos(theta), sin(theta)
    float ct2, st2;       // cos(next theta), sin(next theta)
    float cp, sp;         // cos(phi), sin(phi)

    auto pi = glm::radians(180.f);

    const float TwoPI = 2.0f * pi;
    const float Delta = 1.0f / float(meshResolution);

    ct2 = 0.0f; st2 = -1.0f;

    /* Create the sphere from bottom to top (like a spiral) as a tristrip */
    for (size_t j = 0; j < meshResolution; ++j) {
        ct = ct2;
        st = st2;

        theta2 = ((j + 1) * Delta - 0.5f) * pi;
        ct2 = cos(theta2);
        st2 = sin(theta2);

        pNor->x = ct;
        pNor->y = st;
        pNor->z = 0.0f;
        *pPos = radius * (*pNor);
        pUV->x = 0.0f;
        pUV->y = j * Delta;
        ++pPos; ++pNor; ++pUV;

        for (size_t i = 0; i < meshResolution + 1; ++i)
        {
            phi = TwoPI * i * Delta;
            cp = cos(phi);
            sp = sin(phi);

            pNor->x = ct2 * cp;
            pNor->y = st2;
            pNor->z = ct2 * sp;
            *pPos = radius * (*pNor);
            pUV->x = i * Delta;
            pUV->y = (j + 1) * Delta;
            ++pPos; ++pNor; ++pUV;

            pNor->x = ct * cp;
            pNor->y = st;
            pNor->z = ct * sp;
            *pPos = radius * (*pNor);
            pUV->x = i * Delta;
            pUV->y = j * Delta;
            ++pPos; ++pNor; ++pUV;
        }

        pNor->x = ct2;
        pNor->y = st2;
        pNor->z = 0.0f;
        *pPos = radius * (*pNor);
        pUV->x = 1.0f;
        pUV->y = 1.0f;
        ++pPos; ++pNor; ++pUV;
    }

    std::vector<PrimitiveVertex> data(m_count);
    for (size_t i = 0; i < m_count; i++) {
        data[i].position = positions[i];
        data[i].normal = normals[i];
        data[i].texcoord = texcoords[i];
    }
    return data;
}

std::vector<PrimitiveVertex> ConeMesh(float height, float radius, size_t res) {

    size_t m_count = 2 * 3 * (res);

    std::vector<glm::vec3> positions;
    std::vector<glm::vec3> normals;
    std::vector<glm::vec2> texcoords;
    // Note : not sure for the uv coords

    positions.resize(m_count);
    normals.resize(m_count);
    texcoords.resize(m_count);

    glm::vec3* pPos = &(positions[0]);
    glm::vec3* pNor = &(normals[0]);
    glm::vec2* pUV = &(texcoords[0]);

    std::vector<glm::vec3> baseVertex(res);

    float theta2;     // next theta angle
    float ct, st;     // cos(theta), sin(theta)
    float ct2, st2;   // cos(next theta), sin(next theta)  

    auto pi = glm::radians(180.f);
    const float TwoPI = 2.0f * pi;
    const float Delta = 1.0f / float(res);

    ct2 = -1.0f; st2 = 0.0f;

    // Structure
    for (size_t i = 0; i < res; ++i)
    {
        ct = ct2;
        st = st2;

        theta2 = ((i + 1) * Delta - 0.5f) * TwoPI;
        ct2 = cos(theta2);
        st2 = sin(theta2);

        *pUV = glm::vec2(i * Delta, 1.0f);
        *pPos = glm::vec3(0.0f);
        ++pPos; ++pUV;

        *pUV = glm::vec2(i * Delta, 0.0f);
        *pPos = glm::vec3(radius * ct, radius * st, -height);
        ++pPos; ++pUV;

        *pUV = glm::vec2((i + 1) * Delta, 0.0f);
        *pPos = glm::vec3(radius * ct2, radius * st2, -height);
        baseVertex[i] = *pPos;
        ++pPos; ++pUV;

        pNor[0] = pNor[1] = pNor[2] = glm::normalize(glm::cross(pPos[-2], pPos[-1]));
        pNor += 3;
    }

    // Adding the first one to loop
    baseVertex.push_back(baseVertex[0]);

    // Base
    for (size_t i = 0; i < baseVertex.size() - 1u; ++i)
    {
        *pNor = glm::vec3(0.0f, 0.0f, -1.0f);
        *pUV = glm::vec2(0.5f, 0.0f); //
        *pPos = glm::vec3(0.0f, 0.0f, -height);
        ++pPos; ++pNor; ++pUV;

        *pNor = glm::vec3(0.0f, 0.0f, -1.0f);
        *pUV = glm::vec2((i + 1) * Delta, 0.0f); //
        *pPos = baseVertex[i + 1];
        ++pPos; ++pNor; ++pUV;

        *pNor = glm::vec3(0.0f, 0.0f, -1.0f);
        *pUV = glm::vec2((i)*Delta, 0.0f); //
        *pPos = baseVertex[i];
        ++pPos; ++pNor; ++pUV;
    }

    std::vector<PrimitiveVertex> data(m_count);
    for (size_t i = 0; i < m_count; i++) {
        data[i].position = positions[i];
        data[i].normal = normals[i];
        data[i].texcoord = texcoords[i];
    }
    return data;
}

// TODO: 이건 단순히 코드와 공식이 안이뻐서 수정을 제안함 
//        https://github.com/caosdoar/spheres
std::vector<PrimitiveVertex> cubeMesh() {
    size_t m_count = 2u * 3u * 6u;

    std::vector<glm::vec3> positions(m_count);
    std::vector<glm::vec3> normals(m_count);
    std::vector<glm::vec2> texcoords(m_count);

    positions.resize(m_count);
    normals.resize(m_count);
    texcoords.resize(m_count);

    const size_t PX = 0u;
    const size_t NX = 6u;
    const size_t PY = 12u;
    const size_t NY = 18u;
    const size_t PZ = 24u;
    const size_t NZ = 30u;

    const glm::vec3 default_normals[] = {
        glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)
    };

    /// POSITIVE-X
    positions[PX + 0u] = positions[PX + 5u] = glm::vec3(1.0f, -1.0f, 1.0f);
    positions[PX + 1u] = glm::vec3(1.0f, -1.0f, -1.0f);
    positions[PX + 2u] = positions[PX + 3u] = glm::vec3(1.0f, 1.0f, -1.0f);
    positions[PX + 4u] = glm::vec3(1.0f, 1.0f, 1.0f);
    normals[PX + 0u] = normals[PX + 1u] = normals[PX + 2u] =
    normals[PX + 3u] = normals[PX + 4u] = normals[PX + 5u] = default_normals[0u];

    /// NEGATIVE-X
    positions[NX + 0u] = positions[NX + 5u] = glm::vec3(-1.0f, -1.0f, -1.0f);
    positions[NX + 1u] = glm::vec3(-1.0f, -1.0f, 1.0f);
    positions[NX + 2u] = positions[NX + 3u] = glm::vec3(-1.0f, 1.0f, 1.0f);
    positions[NX + 4u] = glm::vec3(-1.0f, 1.0f, -1.0f);
    normals[NX + 0u] = normals[NX + 1u] = normals[NX + 2u] =
    normals[NX + 3u] = normals[NX + 4u] = normals[NX + 5u] = -default_normals[0u];

    /// POSITIVE-Y
    positions[PY + 0u] = positions[PY + 5u] = glm::vec3(-1.0f, 1.0f, 1.0f);
    positions[PY + 1u] = glm::vec3(1.0f, 1.0f, 1.0f);
    positions[PY + 2u] = positions[PY + 3u] = glm::vec3(1.0f, 1.0f, -1.0f);
    positions[PY + 4u] = glm::vec3(-1.0f, 1.0f, -1.0f);
    normals[PY + 0u] = normals[PY + 1u] = normals[PY + 2u] =
    normals[PY + 3u] = normals[PY + 4u] = normals[PY + 5u] = default_normals[1u];

    /// NEGATIVE-Y
    positions[NY + 0u] = positions[NY + 5u] = glm::vec3(-1.0f, -1.0f, -1.0f);
    positions[NY + 1u] = glm::vec3(1.0f, -1.0f, -1.0f);
    positions[NY + 2u] = positions[NY + 3u] = glm::vec3(1.0f, -1.0f, 1.0f);
    positions[NY + 4u] = glm::vec3(-1.0f, -1.0f, 1.0f);
    normals[NY + 0u] = normals[NY + 1u] = normals[NY + 2u] =
    normals[NY + 3u] = normals[NY + 4u] = normals[NY + 5u] = -default_normals[1u];

    /// POSITIVE-Z
    positions[PZ + 0u] = positions[PZ + 5u] = glm::vec3(-1.0f, -1.0f, 1.0f);
    positions[PZ + 1u] = glm::vec3(1.0f, -1.0f, 1.0f);
    positions[PZ + 2u] = positions[PZ + 3u] = glm::vec3(1.0f, 1.0f, 1.0f);
    positions[PZ + 4u] = glm::vec3(-1.0f, 1.0f, 1.0f);
    normals[PZ + 0u] = normals[PZ + 1u] = normals[PZ + 2u] =
    normals[PZ + 3u] = normals[PZ + 4u] = normals[PZ + 5u] = default_normals[2u];

    /// NEGATIVE-Z
    positions[NZ + 0u] = positions[NZ + 5u] = glm::vec3(1.0f, -1.0f, -1.0f);
    positions[NZ + 1u] = glm::vec3(-1.0f, -1.0f, -1.0f);
    positions[NZ + 2u] = positions[NZ + 3u] = glm::vec3(-1.0f, 1.0f, -1.0f);
    positions[NZ + 4u] = glm::vec3(1.0f, 1.0f, -1.0f);
    normals[NZ + 0u] = normals[NZ + 1u] = normals[NZ + 2u] =
    normals[NZ + 3u] = normals[NZ + 4u] = normals[NZ + 5u] = -default_normals[2u];

    texcoords[PX + 0u] = texcoords[NX + 0u] = texcoords[PY + 0u] = texcoords[NY + 0u] = texcoords[PZ + 0u] = texcoords[NZ + 0u] = glm::vec2(1.0f, 0.f);
    texcoords[PX + 1u] = texcoords[NX + 1u] = texcoords[PY + 1u] = texcoords[NY + 1u] = texcoords[PZ + 1u] = texcoords[NZ + 1u] = glm::vec2(0.0f, 0.f);
    texcoords[PX + 2u] = texcoords[NX + 2u] = texcoords[PY + 2u] = texcoords[NY + 2u] = texcoords[PZ + 2u] = texcoords[NZ + 2u] = glm::vec2(0.0f, 1.f);
    texcoords[PX + 3u] = texcoords[NX + 3u] = texcoords[PY + 3u] = texcoords[NY + 3u] = texcoords[PZ + 3u] = texcoords[NZ + 3u] = glm::vec2(0.0f, 1.f);
    texcoords[PX + 4u] = texcoords[NX + 4u] = texcoords[PY + 4u] = texcoords[NY + 4u] = texcoords[PZ + 4u] = texcoords[NZ + 4u] = glm::vec2(1.0f, 1.f);
    texcoords[PX + 5u] = texcoords[NX + 5u] = texcoords[PY + 5u] = texcoords[NY + 5u] = texcoords[PZ + 5u] = texcoords[NZ + 5u] = glm::vec2(1.0f, 0.f);

    std::vector<PrimitiveVertex> data(m_count);
    for (size_t i = 0; i < m_count; i++) {
        data[i].position = positions[i];
        data[i].normal = normals[i];
        data[i].texcoord = texcoords[i];
    }
    return data;
}
