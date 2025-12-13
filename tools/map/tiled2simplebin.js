/// <reference types="@mapeditor/tiled-api" />
/**
 * Tiled Binary Export Plugin Structure
 * * Target Binary Layout:
 * -----------------------------------------------------
 * HEADER: 
 * - Num Tile Layers (U16)
 * - Num Object Layers (U16)
 * * TILE LAYERS (N times):
 * - Width (U16)
 * - Height (U16)
 * - GIDs array (U16 * Width * Height)
 * * OBJECT LAYERS (M times):
 * - Num Objects (U16)
 * - Objects (K times): 
 * - TYPE (U16, from obj.properties.TYPE)
 * - X (U16, tile unit)
 * - Y (U16, tile unit)
 * -----------------------------------------------------
 */
let littleEndian = true; // Define Endianness for binary writes

/**
 * Calculates the exact size required for the ArrayBuffer
 * based on the map's content and the binary specification.
 * * @param {Tiled.Map} map The Tiled map object.
 * @returns {number} The total size in bytes.
 */
function calculateTotalSize(map) {
	tiled.log(`calculateTotalSize`);
	let size = 0;

	// 1. Header Size: NumLayers (2) + NumObjectLayers (2)
	size += 4; 

	const tileLayers = map.layers.filter(l => l.isTileLayer);
	const objectLayers = map.layers.filter(l => l.isObjectLayer);
	
	// 2. Tile Layers Size
	for (let layer of tileLayers) {
		// Layer Header: Width (2) + Height (2)
		size += 4; 
		
		// GID Data: 2 bytes per tile * width * height
		size += layer.width * layer.height * 2; 
	}

	// 3. Object Layers Size
	for (let layer of objectLayers) {
		// Object Layer Header: Num Objects (2)
		size += 2;

		// Object Data: TYPE (2) + X (2) + Y (2) = 6 bytes per object
		size += layer.objects.length * 6; 
	}
	return size;
}

/**
 * Executes the binary export and writes data to an ArrayBuffer.
 * * @param {Tiled.Map} map The Tiled map object.
 * @returns {ArrayBuffer} The complete binary data buffer.
 */
function exportBinary(map) {
    const totalSize = calculateTotalSize(map);
    const buffer = new ArrayBuffer(totalSize);
    const view = new DataView(buffer);
    let offset = 0; // Tracks the current write position

    const tileLayers = map.layers.filter(l => l.isTileLayer);
    const objectLayers = map.layers.filter(l => l.isObjectLayer);
    
    tiled.log(`Allocating buffer size: ${totalSize} bytes`);

    // ===================================
    // 1. HEADER
    // ===================================
    tiled.log(`Writing header`);
    // Num Tile Layers (U16)
    view.setUint16(offset, tileLayers.length, littleEndian);
    offset += 2;
    
    // Num Object Layers (U16)
    view.setUint16(offset, objectLayers.length, littleEndian);
    offset += 2;
	tiled.log(`Writed header`);


    // ===================================
    // 2. TILE LAYERS
    // ===================================
	tiled.log(`Writing layers`);
    for (let layer of tileLayers) {
        // Layer Width (U16)
        view.setUint16(offset, layer.width, littleEndian);
        offset += 2;
        
        // Layer Height (U16)
        view.setUint16(offset, layer.height, littleEndian);
        offset += 2;
        
        // Tiles (GIDs - U16 array)
		tiled.log(`Writing layer data`);
		for (let y = 0; y < layer.height; y++) {
			for (let x = 0; x < layer.width; x++) {
				let gid = layer.cellAt(x, y).tileId;
				// Write the GID (Global Tile ID). 
				// Note: Truncates GIDs > 65535, as requested by U16 type.
				view.setUint16(offset, gid, littleEndian);
				offset += 2;
			}
		}
    }
	tiled.log(`Writed layers`);

    // ===================================
    // 3. OBJECT LAYERS (TYPE and TILE COORDS)
    // ===================================
	tiled.log(`Writing objects`);
    for (let layer of objectLayers) {
        // Num Objects in Layer (U16)
        view.setUint16(offset, layer.objects.length, littleEndian);
        offset += 2;

        for (let obj of layer.objects) {
            // A. Object TYPE (U16) from properties
            // If the TYPE property is missing or null, default to 0.
            const objectType = obj.properties && obj.properties.TYPE ? obj.properties.TYPE : 0;
            view.setUint16(offset, objectType, littleEndian);
            offset += 2;
            
            // B. Object X (U16) converted to tile units
            const tileX = Math.floor(obj.x / map.tileWidth);
            view.setUint16(offset, tileX, littleEndian);
            offset += 2;
            
            // C. Object Y (U16) converted to tile units
            const tileY = Math.floor(obj.y / map.tileHeight);
            view.setUint16(offset, tileY, littleEndian);
            offset += 2;
        }
    }
	tiled.log(`Writed objects`);

    // --- Final Check ---
    if (offset !== totalSize) {
        throw new Error(`Critical Write Error: Final offset (${offset}) does not match calculated size (${totalSize}).`);
    }

    return buffer; 
}

let Tiled2SimpleBinExporter = {
    name: "Simple Bin Exporter",
    extension: "map",
    description: "Export to a simple binary file with layers and objects",
    version: "1.0",
	write: function(map, fileName) {
		try {
			const buffer = exportBinary(map);
			// Write the ArrayBuffer to disk
			let file = new BinaryFile(fileName, BinaryFile.WriteOnly);
			file.write(buffer);
			file.commit();
			return "";
		} catch (e) {
			return e.toString();
		}
	}
};

tiled.registerMapFormat("Custom Binary (*.map)", Tiled2SimpleBinExporter);
