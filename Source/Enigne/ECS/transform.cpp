#include "Engine/ECS/transform.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>

using namespace glex;

Transform::Transform() :
	m_parent(nullptr), m_child(nullptr), m_prev(nullptr), m_next(nullptr),
	m_position(0.0f, 0.0f, 0.0f), m_scale(1.0f, 1.0f, 1.0f), m_rotation(1.0f, 0.0f, 0.0f, 0.0f),
	m_flags(DIRTY_FLAG_ALL) {}

void Transform::SetDirtyFlag()
{
	if (m_flags & FLAG_LOCK)
		return;
	m_flags = DIRTY_FLAG_ALL;
	for (Transform* p = m_child; p != nullptr; p = p->m_next)
		p->SetDirtyFlag();
}

void Transform::SetChildrenDirty()
{
	for (Transform* p = m_child; p != nullptr; p = p->m_next)
		p->SetDirtyFlag();
}

void Transform::SetParent(Transform* parent)
{
	if (m_parent != nullptr)
	{
		if (m_parent->m_child == this)
			parent->m_child = m_next;
		if (m_prev != nullptr)
			m_prev->m_next = m_next;
		if (m_next != nullptr)
			m_next->m_prev = m_prev;
		m_prev = nullptr;
		m_next = nullptr;
	}
	m_parent = parent;
	if (parent != nullptr)
	{
		Transform* head = parent->m_child;
		if (head != nullptr)
			head->m_prev = this;
		m_next = head;
		parent->m_child = this;
	}
	SetDirtyFlag();
}

void Transform::SetGlobalPosition(glm::vec3 const& pos)
{
	if (m_parent == nullptr)
		SetPosition(pos);
	else
	{
		m_position = (pos - m_parent->GetGlobalPosition()) * glm::inverse(m_parent->GetGlobalRotation()) / m_parent->GetGlobalScale();
		SetDirtyFlag();
	}
}

void Transform::SetGlobalRotation(glm::quat const& rot)
{
	if (m_parent == nullptr)
		SetRotation(rot);
	else
	{
		m_rotation = rot * glm::inverse(m_parent->GetGlobalRotation());
		SetDirtyFlag();
	}
}

void Transform::LockAt(glm::vec3 const& pos, glm::quat const& rot)
{
	m_globalPosition = pos;
	m_globalRotation = rot;
	m_flags = FLAG_LOCK;
	// Yes, children keep their RELATIVE transform unless they are physics objects, too.
	SetChildrenDirty();
}

void Transform::Unlock()
{
	if (m_parent == nullptr)
	{
		m_position = m_globalPosition;
		m_rotation = m_globalRotation;
	}
	else
	{
		m_position = (m_globalPosition - m_parent->GetGlobalPosition()) * glm::inverse(m_parent->GetGlobalRotation()) / m_parent->GetGlobalScale();
		m_rotation = m_globalRotation * glm::inverse(m_parent->GetGlobalRotation());
	}
	m_flags = DIRTY_FLAG_MATRIX;
}

void Transform::FlushGlobal() const
{
	if (m_flags & DIRTY_FLAG_GLOBAL)
	{
		if (m_parent != nullptr)
		{
			glm::quat parentRotation = m_parent->GetGlobalRotation();
			glm::vec3 parentPosition = m_parent->GetGlobalPosition();
			glm::vec3 parentScale = m_parent->GetGlobalScale();
			m_globalRotation = m_rotation * parentRotation;
			m_globalScale = m_scale * parentScale;
			m_globalPosition = parentPosition + m_position * parentScale * parentRotation;
		}
		else
		{
			m_globalPosition = m_position;
			m_globalRotation = m_rotation;
			m_globalScale = m_scale;
		}
		m_flags &= ~DIRTY_FLAG_GLOBAL;
	}
}

glm::vec3 const& Transform::GetGlobalPosition() const
{
	FlushGlobal();
	return m_globalPosition;
}

glm::quat const& Transform::GetGlobalRotation() const
{
	FlushGlobal();
	return m_globalRotation;
}

glm::vec3 const& Transform::GetGlobalScale() const
{
	FlushGlobal();
	return m_globalScale;
}

glm::mat4 const& Transform::GetModelMat() const
{
	// Model matrix is dirty.
	if (m_flags & DIRTY_FLAG_MATRIX)
	{
		FlushGlobal();
		m_modelMat = glm::mat4(1.0f);
		m_modelMat[0][0] *= m_globalScale.x;
		m_modelMat[1][1] *= m_globalScale.y;
		m_modelMat[2][2] *= m_globalScale.z;
		m_modelMat = glm::mat4_cast(m_globalRotation) * m_modelMat;
		m_modelMat[3] = glm::vec4(m_globalPosition, 1.0f);
		m_flags &= ~DIRTY_FLAG_MATRIX;
	}
	return m_modelMat;
}

glm::vec3 Transform::Forward() const
{
	return glm::rotate(m_rotation, glm::vec3(0.0f, 0.0f, 1.0f));
}

glm::vec3 Transform::Left() const
{
	return glm::rotate(m_rotation, glm::vec3(1.0f, 0.0f, 0.0f));
}

void Transform::SetPosition(glm::vec3 const& pos)
{
	m_position = pos;
	SetDirtyFlag();
}

void Transform::SetRotation(glm::quat const& rot)
{
	m_rotation = rot;
	SetDirtyFlag();
}

void Transform::SetRotation(glm::vec3 const& euler)
{
	m_rotation = glm::quat(euler);
	SetDirtyFlag();
}

void Transform::SetScale(glm::vec3 const& scale)
{
	m_scale = scale;
	SetDirtyFlag();
}

void Transform::LookTowrards(glm::vec3 const& direction)
{
	m_rotation = glm::quat(glm::vec3(0.0f, 0.0f, 1.0f), glm::normalize(direction));
	SetDirtyFlag();
}

void Transform::Rotate(glm::vec3 const& axis, float angle)
{
	m_rotation = glm::rotate(m_rotation, angle, axis);
	SetDirtyFlag();
}