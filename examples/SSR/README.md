第一遍渲染（shadow pass）：获得 `shadow map` 

* 摄像机：方向光，正交投影
* 输出：`shadow_map` 



第二遍渲染（geometry pass）：获得场景的几何，材质信息

- 摄像机：摄像机，透视投影
- 输出：
  - gbuffer: position in view space
  - gbuffer: normal in view space
  - gbuffer: materiali - diffuse



第三遍渲染（color pass）：每个点反射的 uv 信息

- 绘制到矩形上
- 输出：最终颜色
