"use strict";

module.exports = class Tool {
    static TOOLS(){
        return {
            'ItemEditTool': 	{ 'Id': 0, 'isItem':  true, 'name': 'ItemEditTool' },
            'APEditTool': 		{ 'Id': 1, 'isItem': false, 'name': 'APEditTool' },
            'AHEditTool': 		{ 'Id': 2, 'isItem':  true, 'name': 'AHEditTool' },
            'APAddTool': 		{ 'Id': 3, 'isItem':  true, 'name': 'APAddTool' },
            'APInsertTool': 	{ 'Id': 4, 'isItem':  true, 'name': 'APInsertTool' },
            'APKnifeTool':		{ 'Id': 5, 'isItem':  true, 'name': 'APKnifeTool' },
            'FigureAddTool':	{ 'Id': 6, 'isItem':  true, 'name': 'FigureAddTool' },
            'GuideAddTool':		{ 'Id': 7, 'isItem':  true, 'name': 'GuideAddTool' },
        };
    }
    
    static getToolFromIndex(index) {
        for (const [key, tool] of Object.entries(Tool.TOOLS())) {
            if (tool.Id == index) {
                return tool;
            }
        }

        console.error('BUG Tool.getToolFromIndex', index);
        return undefined;
    }
}