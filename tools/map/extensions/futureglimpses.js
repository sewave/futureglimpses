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
 * - Title length (U16)
 * - Title string (N times)
 * - Description length (U16)
 * - Description string (N times)
 * - Player Gold (U32)
 * - Player Wood (U32)
 * - Computer Gold (U32)
 * - Computer Wood (U32)
 * - Enable barracks (U8)
 * - Enable blacksmith (U8)
 * - Enable farm (U8)
 * - Enable stables (U8)
 * - Enable tower (U8)
 * - IA Mode (U8)
 * - Peace time (U16)
 * * OBJECT LAYERS (M times):
 * - Num Objects (U16)
 * - Objects (24 * K times): 
 * - UNIT_TYPE (U8, from obj.properties.UNIT_TYPE)
 * - UNIT_CONTROLLER (U8, from obj.properties.UNIT_CONTROLLER)
 * - X (U16, tile unit)
 * - Y (U16, tile unit)
 * - isCustom (U8)
 * - name (11 * U8)
 * - maxHealth (U16)
 * - minDamage (U8)
 * - maxDamage (U8)
 * - mustSurvive (U8)
 * - armor (U8)
 * - padding (3 * U8) to align with C structs
 * -----------------------------------------------------
 */
let littleEndian = true; // Define Endianness for binary writes
const OBJECT_DATA_SIZE = 28;

function getUtf8Size(str) {
    var size = 0;
    for (var i = 0; i < str.length; i++) {
        var code = str.charCodeAt(i);
        if (code < 0x80) size += 1;
        else if (code < 0x800) size += 2;
        else if (code < 0xD800 || code >= 0xE000) size += 3;
        else {
            i++;
            size += 4;
        }
    }
    return size;
}

function writeUtf8String(view, offset, str) {
    for (var i = 0; i < str.length; i++) {
        var code = str.charCodeAt(i);
        if (code < 0x80) {
            view.setUint8(offset++, code);
        } else if (code < 0x800) {
            view.setUint8(offset++, 0xC0 | (code >> 6));
            view.setUint8(offset++, 0x80 | (code & 0x3F));
        } else if (code < 0xD800 || code >= 0xE000) {
            view.setUint8(offset++, 0xE0 | (code >> 12));
            view.setUint8(offset++, 0x80 | ((code >> 6) & 0x3F));
            view.setUint8(offset++, 0x80 | (code & 0x3F));
        } else {
            i++;
            var codePoint = ((code & 0x3FF) << 10) | (str.charCodeAt(i) & 0x3FF);
            view.setUint8(offset++, 0xF0 | (codePoint >> 18));
            view.setUint8(offset++, 0x80 | ((codePoint >> 12) & 0x3F));
            view.setUint8(offset++, 0x80 | ((codePoint >> 6) & 0x3F));
            view.setUint8(offset++, 0x80 | (codePoint & 0x3F));
        }
    }
    return offset;
}

/**
 * Calculates the exact size required for the ArrayBuffer
 * based on the map's content and the binary specification.
 * @param {Tiled.Map} map The Tiled map object.
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

        size += layer.objects.length * OBJECT_DATA_SIZE;
	}

	// 4 resources of 32 bits
	size += 4 * 4;
    // Other attributes
    size += 8;

    var mapProperties = map.resolvedProperties();

    var titleLength = getUtf8Size(mapProperties.MSG_TITLE || "");
    tiled.log(`Title length: ${titleLength}`);
    size += 2 + titleLength; // Title length (2) + title string bytes

    var descriptionLength = getUtf8Size(mapProperties.MSG_DESCRIPTION || "");
    tiled.log(`Description length: ${descriptionLength}`);
    size += 2 + descriptionLength; // Description length (2) + description string bytes

    var msgWinLength = getUtf8Size(mapProperties.MSG_WIN || "");
    tiled.log(`Win length: ${msgWinLength}`);
    size += 2 + msgWinLength;

    var msgLoseLength = getUtf8Size(mapProperties.MSG_LOSE || "");
    tiled.log(`Lose length: ${msgLoseLength}`);
    size += 2 + msgLoseLength;

	return size;
}

/**
 * Writes the binary data for a single object to the DataView.
 * @param {DataView} view The DataView to write to.
 * @param {number} offset The offset to start writing at.
 * @param {Tiled.Object} obj The object to write.
 * @param {Tiled.Map} map The Tiled map object.
 */
function writeObjectData(view, offset, obj, map) {
    // A. Object UNIT_TYPE (U8) from properties
    // If the UNIT_TYPE property is missing or null, default to 0.
    const resolvedProperties = obj.resolvedProperties();

    const objectType = resolvedProperties.UNIT_TYPE.value;
    view.setUint8(offset, objectType, littleEndian);
    offset += 1;

    // B. Object UNIT_CONTROLLER (U8) from properties
    // If the UNIT_CONTROLLER property is missing or null, default to 0.
    const objectController = resolvedProperties.UNIT_CONTROLLER.value;
    view.setUint8(offset, objectController, littleEndian);
    offset += 1;

    // C. Object X (U16) converted to tile units
    const tileX = Math.floor(obj.x / map.tileWidth);
    view.setUint16(offset, tileX, littleEndian);
    offset += 2;

    // D. Object Y (U16) converted to tile units
    const tileY = Math.floor(obj.y / map.tileHeight);
    view.setUint16(offset, tileY, littleEndian);
    offset += 2;

    // E. Is a custom object?
    const isCustom = resolvedProperties.CUSTOM || false;
    tiled.log(`Custom: [` + isCustom + `]`);
    view.setUint8(offset, isCustom, littleEndian);
    offset += 1;

    // F. Name, up to 10 bytes + end
    var name = obj.name ? obj.name || "" : "";
    while (getUtf8Size(name) > 10) {
        name = name.substring(0, name.length - 1);
    }
    const nameLength = getUtf8Size(name);
    tiled.log(`Name length: [` + nameLength + `]`);
    while (getUtf8Size(name) < 10) {
        name = name.padEnd(name.length + 1);
    }
    tiled.log(`Name: [` + name + `]`);
    writeUtf8String(view, offset, name);
    offset += nameLength;
    // Finish string with 0es
    for (let i = nameLength; i < 11; i++) {
        view.setUint8(offset++, 0, littleEndian);
    }

    const maxHealth = resolvedProperties.MAX_HEALTH ? resolvedProperties.MAX_HEALTH : 0;
    tiled.log(`maxHealth: [` + maxHealth + `]`);
    view.setUint16(offset, maxHealth, littleEndian);
    offset += 2;

    const minDamage = resolvedProperties.MIN_DAMAGE ? resolvedProperties.MIN_DAMAGE : 0;
    tiled.log(`minDamage: [` + minDamage + `]`);
    view.setUint8(offset, minDamage, littleEndian);
    offset += 1;

    const maxDamage = resolvedProperties.MAX_DAMAGE ? resolvedProperties.MAX_DAMAGE : 0;
    tiled.log(`minDamage: [` + maxDamage + `]`);
    view.setUint8(offset, maxDamage, littleEndian);
    offset += 1;

    const mustSurvive = resolvedProperties.MUST_SURVIVE || false;
    tiled.log(`Must Survive: [` + mustSurvive + `]`);
    view.setUint8(offset, mustSurvive, littleEndian);
    offset += 1;

    const armor = resolvedProperties.ARMOR || 0;
    tiled.log(`Armor: [` + armor + `]`);
    view.setUint8(offset, armor, littleEndian);
    offset += 1;

    // Must pad to 32 bit size for C struct alignment
    offset += 3;
}

function writeStringAndLength(view, offset, name, string) {
    var length = getUtf8Size(string);
    // Write Title Length (U16)
    view.setUint16(offset, length, littleEndian);
    offset += 2;
    // Write Title String (UTF-8 bytes)
    writeUtf8String(view, offset, string);
    offset += length;
    tiled.log(`Writed ${name} with length ${length}`);
    return length + 2;
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
            writeObjectData(view, offset, obj, map);
            offset += OBJECT_DATA_SIZE;
        }
    }
	tiled.log(`Writed objects`);

    // ===================================
    // 4. MAP ATTRIBUTES (MSG_TITLE and MSG_DESCRIPTION)
    // ===================================
    tiled.log(`Writing map attributes`);
    var mapProperties = map.resolvedProperties();

    // Map strings
    offset += writeStringAndLength(view, offset, "title", mapProperties.MSG_TITLE || "");
    offset += writeStringAndLength(view, offset, "description", mapProperties.MSG_DESCRIPTION || "");
    offset += writeStringAndLength(view, offset, "win", mapProperties.MSG_WIN || "");
    offset += writeStringAndLength(view, offset, "lose", mapProperties.MSG_LOSE || "");

    // Write resources
    var resGoldPlayer = mapProperties.RES_GOLD_PLAYER || 0;
    var resWoodPlayer = mapProperties.RES_WOOD_PLAYER || 0;
    var resGoldComputer = mapProperties.RES_GOLD_COMPUTER || 0;
    var resWoodComputer = mapProperties.RES_WOOD_COMPUTER || 0;
    view.setUint32(offset, resGoldPlayer, littleEndian);
    tiled.log(`Writed resources for player gold: ${resGoldPlayer}`);
    offset += 4;
    view.setUint32(offset, resWoodPlayer, littleEndian);
    tiled.log(`Writed resources for player wood: ${resWoodPlayer}`);
    offset += 4;
    view.setUint32(offset, resGoldComputer, littleEndian);
    tiled.log(`Writed resources for computer gold: ${resGoldComputer}`);
    offset += 4;
    view.setUint32(offset, resWoodComputer, littleEndian);
    tiled.log(`Writed resources for computer wood: ${resWoodComputer}`);
    offset += 4;

    var enableBarracks = mapProperties.ENABLE_BARRACKS || false;
    var enableBlacksmith = mapProperties.ENABLE_BLACKSMITH || false;
    var enableFarm = mapProperties.ENABLE_FARM || false;
    var enableStables = mapProperties.ENABLE_STABLES || false;
    var enableTower = mapProperties.ENABLE_TOWER || false;
    var aiMode = mapProperties.AI_MODE ? mapProperties.AI_MODE.value : 0;
    var peaceTime = mapProperties.PEACE_TIME || 0;
    view.setUint8(offset++, enableBarracks, littleEndian);
    tiled.log(`Writed enableBarracks: ${enableBarracks}`);
    view.setUint8(offset++, enableBlacksmith, littleEndian);
    tiled.log(`Writed enableBlacksmith: ${enableBlacksmith}`);
    view.setUint8(offset++, enableFarm, littleEndian);
    tiled.log(`Writed enableFarm: ${enableFarm}`);
    view.setUint8(offset++, enableStables, littleEndian);
    tiled.log(`Writed enableStables: ${enableStables}`);
    view.setUint8(offset++, enableTower, littleEndian);
    tiled.log(`Writed enableTower: ${enableTower}`);
    view.setUint8(offset++, aiMode, littleEndian);
    tiled.log(`Writed aiMode: ${aiMode}`);
    view.setUint16(offset, peaceTime, littleEndian);
    tiled.log(`Writed peaceTime: ${peaceTime}`);
    offset += 2;

    tiled.log(`Writed map attributes`);

    // --- Final Check ---
    if (offset !== totalSize) {
        throw new Error(`Critical Write Error: Final offset (${offset}) does not match calculated size (${totalSize}).`);
    }

    return buffer;
}

let Tiled2FutureGlimpsesExporter = {
    name: "Future Glimpses map",
    extension: "fgm",
    description: "Export to a simple binary file with layers, objects, and some attributes",
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

tiled.registerMapFormat("Future Glimpses map (*.fgm)", Tiled2FutureGlimpsesExporter);
