#!/usr/bin/env python

# glassy armadillo, happy buddha and horse
# Copyright (c) 2011-2012 Hiroshi Tsubokawa

import fujiyama

si = fujiyama.SceneInterface()

#plugins
si.OpenPlugin('ConstantShader')
si.OpenPlugin('PlasticShader')
si.OpenPlugin('GlassShader')

#Camera
si.NewCamera('cam1', 'PerspectiveCamera')
si.SetProperty3('cam1', 'translate', 0, 1.5, 7)

#Light
si.NewLight('light1', 'PointLight')
si.SetProperty3('light1', 'position', 5, 12, -5)

#Shader
si.NewShader('armadillo_shader', 'GlassShader')
si.SetProperty3('armadillo_shader', 'filter_color', .75, .025, .4)

si.NewShader('happy_shader', 'GlassShader')
si.SetProperty3('happy_shader', 'filter_color', .4, .75, .025)

si.NewShader('horse_shader', 'GlassShader')
si.SetProperty3('horse_shader', 'filter_color', .025, .4, .75)

si.NewShader('dome_shader', 'ConstantShader')
si.NewShader('floor_shader', 'PlasticShader')

#Mesh
si.NewMesh('armadillo_mesh', '../../mesh/armadillo.mesh')
si.NewMesh('happy_mesh', '../../mesh/happy.mesh')
si.NewMesh('horse_mesh', '../../mesh/horse.mesh')
si.NewMesh('dome_mesh', '../../mesh/dome.mesh')
si.NewMesh('floor_mesh', '../../mesh/floor.mesh')

#ObjectInstance
si.NewObjectInstance('armadillo1', 'armadillo_mesh')
si.AssignShader('armadillo1', 'armadillo_shader')
si.SetProperty3('armadillo1', 'translate', -2, 0, -3)

si.NewObjectInstance('happy1', 'happy_mesh')
si.AssignShader('happy1', 'happy_shader')

si.NewObjectInstance('horse1', 'horse_mesh')
si.AssignShader('horse1', 'horse_shader')
si.SetProperty3('horse1', 'translate', 2, 0, -3)

si.NewObjectInstance('dome1', 'dome_mesh')
si.AssignShader('dome1', 'dome_shader')

si.NewObjectInstance('floor1', 'floor_mesh')
si.AssignShader('floor1', 'floor_shader')

#FrameBuffer
si.NewFrameBuffer('fb1', 'rgba')

#Renderer
si.NewRenderer('ren1')
si.AssignCamera('ren1', 'cam1')
si.AssignFrameBuffer('ren1', 'fb1')
si.SetProperty2('ren1', 'resolution', 640, 480)
#si.SetProperty2('ren1', 'resolution', 160, 120)
si.SetProperty1('ren1', 'max_refract_depth', 10)
si.SetProperty1('ren1', 'cast_shadow', 0)

#Rendering
si.RenderScene('ren1')

#Output
si.SaveFrameBuffer('fb1', '../glassy_happy.fb')

#Run commands
si.Run()
#si.Print()

