///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2011 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/** ***************************************************************************
*** \file    map_view.h
*** \author  Philip Vorsilak, gorzuate@allacrost.org
*** \author  Tyler Olsen, roots@allacrost.org
*** \brief   Header file for the map view widget
*** **************************************************************************/

#ifndef __MAP_VIEW_HEADER__
#define __MAP_VIEW_HEADER__

#include <QGraphicsScene>
#include <QStringList>
#include <QTreeWidgetItem>

#include "map_data.h"
#include "tileset.h"

namespace hoa_editor {

/** ***************************************************************************
*** \brief The GUI component where map tiles are drawn and edited
***
*** This class draws all of the tiles that compose the map to the editor's main window screen.
*** All of the data for the map is stored in the MapData object that the class maintains a pointer
*** to. Some editor properties, such as whether or not the tile grid is visible, are stored here.
*** **************************************************************************/
class MapView : public QGraphicsScene {
	Q_OBJECT // Macro needed to use QT's slots and signals

public:
	/** \param parent The parent widget of the grid, which should be the main editor window
	*** \param data A pointer to the map data to draw
	**/
	MapView(QWidget* parent, MapData* data);

	~MapView();

	//! \name Class member accessor functions
	//@{
	QGraphicsView* GetGraphicsView() const
		{ return _graphics_view; }

	void SetGridVisible(bool value)
		{ _grid_visible = value; DrawMap(); }

	void SetSelectionVisible(bool value)
		{ _selection_visible = value; DrawMap(); }

	bool ToggleGridVisible()
		{ _grid_visible = !_grid_visible; return _grid_visible; }

	bool ToggleSelectionVisible()
		{ _selection_visible = !_selection_visible; return _selection_visible; }

	void SetEditMode(TILE_EDIT_MODE new_mode)
		{ _tile_mode = new_mode; }
	//@}

	/** \brief Creates a new context for each layer.
	*** \param inherit_context The index of the context to inherit from.
	**/
	void CreateNewContext(uint32 inherit_context);

	//! \brief Clears all data from _select_layer by filling it with NO_TILE
	void ClearSelectionLayer()
		{ _select_layer.ClearLayer(); }

	/** \name Map Modification Functions (Right-Click)
	*** \brief Functions to insert or delete rows or columns of tiles from the map.
	*** \param tile_index An ID (range: {0, length * height - 1}) of the tile
	***        used to determine the row or column upon which to perform the
	***        operation.
	***
	*** \note This feature is accessed by right-clicking on the map. It could
	***       be used elsewhere if the proper tile index is passed as a
	***       parameter.
	**/
	//{@
	void InsertTileRow(uint32 tile_index);
	void InsertTileCol(uint32 tile_index);
	void DeleteTileRow(uint32 tile_index);
	void DeleteTileCol(uint32 tile_index);
	//@}

	//! \brief Draws the entire map to the tile grid area
    void DrawMap();

	/** \brief The names of each individual context
	*** \note Maximum size is 32 entries, the maximum amount of contexts that a single map supports.
	**/
	QStringList context_names;

	//! \brief A list storing the background music filenames.
	QStringList music_files;

protected:
	/** \name User Input Event Processing Functions
	*** \brief Functions to process mouse and keyboard events on the map
	*** \param event A pointer to the type of QEvent that was generated
	**/
	//{@
	void mousePressEvent(QGraphicsSceneMouseEvent* event);
	void mouseMoveEvent(QGraphicsSceneMouseEvent* event);
	void mouseReleaseEvent(QGraphicsSceneMouseEvent* event);
	void contextMenuEvent(QGraphicsSceneContextMenuEvent* event);
	void keyPressEvent(QKeyEvent* event);
	//@}

private slots:
	/** \name Map Modification Functions (Right-Click)
	*** \brief Functions available from right-clicking a tile on the map vie
	**/
	//{@
	void _InsertTileRow();
	void _InsertTileColumn();
	void _DeleteTileRow();
	void _DeleteTileColumn();
	//@}

private:
	//! \brief A pointer to the underlying map data to read and manipulate
	MapData* _map_data;

	//! \brief When TRUE the grid between tiles is displayed.
	bool _grid_visible;

	//! \brief When TRUE the rectangle of chosen tiles is displayed.
	bool _selection_visible;

	/** \name Tile Coordinates
	*** These members constitute the x and y (column and row) coorindates of a tile. The coordinates are
	*** used for various actions, such as updating the status bar of the main window or moving tiles from
	*** a source to a destination.
	***     - _cursor_tile_x/y:           The tile that the mouse cursor currently points to
	***     - _move_source_tile_x/y:      The location of the source when moving a tile to a new location
	***     - _selection_start_tile_x/y:  The starting tile when selecting multiple tiles together
	**/
	//@{
	int32 _cursor_tile_x;
	int32 _cursor_tile_y;
	int32 _move_source_tile_x;
	int32 _move_source_tile_y;
	int32 _selection_start_tile_x;
	int32 _selection_start_tile_y;
	//@}

	//! \brief The current tile editing tool that is active
	TILE_EDIT_MODE _tile_mode;

	/** \brief A tile layer used for indicating a selected area of a tile layer
	*** This data exists only in the editor and is not a part of the map file. It acts similar to an
	*** actual tile layer as far as drawing is concerned, and is not managed by _data_model.
	**/
	TileLayer _select_layer;

	//! \brief Menu for right-clicks events on the map
    QMenu* _right_click_menu;

	/** \name Right-click Menu Actions
	*** \brief Correspond to the private slots functions for user actions
	**/
	//{@
	QAction* _insert_row_action;
	QAction* _insert_column_action;
	QAction* _delete_row_action;
	QAction* _delete_column_action;
	//@}

	//! \brief A one-tile sized square used to highlight multi-tile selections (colored blue at 50% opacity)
	QPixmap _selection_tile;

	//! \brief Used to display the graphics widgets
	QGraphicsView* _graphics_view;

	/** \brief Paints the currently selected tileset tile at a location on the map
	*** \param x The x coordinate of the tile to paint to
	*** \param y The y coordinate of the tile to paint to
	**/
	void _PaintTile(uint32 x, uint32 y);

	/** \brief Deletes the tile at the chosen location on the map
	*** \param x The x coordinate of the tile to delete
	*** \param y The y coordinate of the tile to delete
	**/
	void _DeleteTile(int32 x, int32 y);

	//! \brief A helper function to DrawMap() that draws the tile grid over the tiles
	void _DrawGrid();
}; // class MapView : public QGraphicsScene

} // namespace hoa_editor

#endif // __MAP_VIEW_HEADER__