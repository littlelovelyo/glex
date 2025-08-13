#pragma once
#include "Engine/GUI/control.h"

namespace glex::ui
{
	class Grid : public Control
	{
		Vector<Vector<UniquePtr<Control>>> m_children;	// Child can be nullptr. Stored column-by-column.
		Vector<int16_t> m_rowStretch, m_columnStretch;	// 0 means fit child's size. + means fixed size. - means proportion.
		Vector<float> m_rowSize, m_columnSize;

		// GetChild cache.
		uint32_t m_numChildren = 0;
		int32_t m_lastGetChild = -1;
		int32_t m_lastRow = -1;
		int32_t m_lastColumn = 0;

	public:
		Grid(uint32_t numRows, uint32_t numColumns);
		virtual glm::vec2 OnMeasure(glm::vec2 availableSize) override;
		virtual void OnArrange() override;
		virtual uint32_t NumChildren() const;
		virtual WeakPtr<Control> GetChild(uint32_t index);
		void SetRowHeight(uint32_t row, int16_t stretch) { m_rowStretch[row] = stretch; InvalidateMeasure(); }
		void SetColumnWidth(uint32_t column, int16_t stretch) { m_columnStretch[column] = stretch; InvalidateMeasure(); }
		UniquePtr<Control> Put(uint32_t row, uint32_t column, UniquePtr<Control> control);
		WeakPtr<Control> Get(uint32_t row, uint32_t column) { return m_children[column][row]; }
	};
}