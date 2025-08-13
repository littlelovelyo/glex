#include "grid.h"
#include <numeric>

using namespace glex;
using namespace glex::ui;

Grid::Grid(uint32_t numRows, uint32_t numColumns)
{
	m_children.resize(numColumns);
	for (auto& row : m_children) row.resize(numRows);
	m_rowStretch.resize(numRows);
	m_columnStretch.resize(numColumns);
	m_rowSize.resize(numRows);
	m_columnSize.resize(numColumns);
}

glm::vec2 Grid::OnMeasure(glm::vec2 availableSize)
{
	// Set fixed size cells.
	for (uint32_t i = 0; i < m_rowStretch.size(); i++)
		m_rowSize[i] = m_rowStretch[i] > 0 ? m_rowStretch[i] : 0.0f;
	for (uint32_t i = 0; i < m_columnStretch.size(); i++)
		m_columnSize[i] = m_columnStretch[i] > 0 ? m_columnStretch[i] : 0.0f;

	// First measure 'auto' cells.
	for (uint32_t c = 0; c < m_children.size(); c++)
	{
		auto& column = m_children[c];
		for (uint32_t r = 0; r < column.size(); r++)
		{
			WeakPtr<Control> cell = column[r];
			if (cell != nullptr)
			{
				glm::vec2 desiredSize = cell->Measure(glm::vec2(m_columnSize[c], m_rowSize[r]));
				if (m_rowStretch[r] == 0 || m_columnStretch[c] == 0)
				{
					if (m_rowStretch[r] == 0)
						m_rowSize[r] = glm::max(m_rowSize[r], desiredSize.y);
					if (m_columnStretch[c] == 0)
						m_columnSize[c] = glm::max(m_columnSize[c], desiredSize.x);
				}
			}
		}
	}

	float rowSize = std::accumulate(m_rowSize.begin(), m_rowSize.end(), 0.0f);
	float columnSize = std::accumulate(m_columnSize.begin(), m_columnSize.end(), 0.0f);
	return glm::vec2(columnSize, rowSize);
}

void Grid::OnArrange()
{
	// Decide proportional cell size
	for (uint32_t i = 0; i < m_rowStretch.size(); i++)
	{
		if (m_rowStretch[i] < 0)
			m_rowSize[i] = 0.0f;
	}
	for (uint32_t i = 0; i < m_columnStretch.size(); i++)
	{
		if (m_columnStretch[i] < 0)
			m_columnSize[i] = 0.0f;
	}

	float rowSize = std::accumulate(m_rowSize.begin(), m_rowSize.end(), 0.0f);
	float columnSize = std::accumulate(m_columnSize.begin(), m_columnSize.end(), 0.0f);
	uint32_t rowSlices = 0, columnSlices = 0;
	for (int16_t v : m_rowStretch)
	{
		if (v < 0)
			rowSlices += -v;
	}
	for (int16_t v : m_columnStretch)
	{
		if (v < 0)
			columnSlices += -v;
	}
	glm::vec2 unit = (m_actualSize - glm::vec2(columnSize, rowSize)) / glm::vec2(columnSlices, rowSlices);
	for (uint32_t i = 0; i < m_rowSize.size(); i++)
	{
		if (m_rowStretch[i] < 0)
			m_rowSize[i] = glm::max(m_rowSize[i], -m_rowStretch[i] * unit.y);
	}
	for (uint32_t i = 0; i < m_columnSize.size(); i++)
	{
		if (m_columnStretch[i] < 0)
			m_columnSize[i] = glm::max(m_columnSize[i], -m_columnStretch[i] * unit.x);
	}

	glm::vec2 pos = glm::vec2(0.0f, 0.0f);
	for (uint32_t c = 0; c < m_children.size(); c++)
	{
		auto& column = m_children[c];
		pos.y = 0.0f;
		for (uint32_t r = 0; r < column.size(); r++)
		{
			WeakPtr<Control> cell = column[r];
			if (cell != nullptr)
				cell->Arrange(pos, glm::vec2(m_columnSize[c], m_rowSize[r]));
			pos.y += m_rowSize[r];
		}
		pos.x += m_columnSize[c];
	}
}

UniquePtr<Control> Grid::Put(uint32_t row, uint32_t column, UniquePtr<Control> control)
{
	UniquePtr<Control> old = std::move(m_children[column][row]);
	if (old != nullptr)
	{
		old->SetParent(nullptr);
		m_numChildren--;
	}
	if (control != nullptr)
	{
		control->SetParent(this);
		m_numChildren++;
	}
	m_children[column][row] = std::move(control);
	InvalidateMeasure();
	m_lastGetChild = -1;
	m_lastRow = -1;
	m_lastColumn = 0;
	return old;
}

uint32_t Grid::NumChildren() const
{
	return m_numChildren;
}

WeakPtr<Control> Grid::GetChild(uint32_t index)
{
	if (m_lastGetChild >= static_cast<int32_t>(index))
	{
		m_lastGetChild = -1;
		m_lastRow = -1;
		m_lastColumn = 0;
	}
	for (; m_lastColumn < m_columnSize.size(); m_lastColumn++)
	{
		for (m_lastRow++; m_lastRow < m_rowSize.size(); m_lastRow++)
		{
			WeakPtr<Control> cell = m_children[m_lastColumn][m_lastRow];
			if (cell != nullptr)
			{
				m_lastGetChild++;
				if (m_lastGetChild == index)
					return cell;
			}
		}
		m_lastRow = -1;
	}
	std::unreachable();
}