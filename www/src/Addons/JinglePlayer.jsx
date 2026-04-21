import React from 'react';

const JinglePlayer = ({ config, updateConfig }) => {
  return (
    <div className="p-4 bg-gray-800 rounded-lg">
      <h2 className="text-xl font-bold mb-4">Jingle Player (UART Mode)</h2>
      
      {/* 有効・無効スイッチ */}
      <div className="flex items-center justify-between mb-4">
        <span>Enabled</span>
        <input 
          type="checkbox" 
          checked={config.enabled}
          onChange={(e) => updateConfig({ enabled: e.target.checked })}
        />
      </div>

      {/* 音量スライダー (0-30) */}
      <div className="flex flex-col">
        <div className="flex justify-between">
          <span>Volume</span>
          <span>{config.volume} / 30</span>
        </div>
        <input 
          type="range" min="0" max="30" 
          value={config.volume}
          onChange={(e) => updateConfig({ volume: parseInt(e.target.value) })}
          className="w-full h-2 bg-gray-600 rounded-lg appearance-none cursor-pointer"
        />
      </div>
    </div>
  );
};

export default JinglePlayer;
