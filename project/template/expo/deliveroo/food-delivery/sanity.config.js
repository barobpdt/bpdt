import {defineConfig} from 'sanity'
import {structureTool} from 'sanity/structure'
import {visionTool} from '@sanity/vision'
import {schemaTypes} from './schemaTypes'

export default defineConfig({
  name: 'default',
  title: 'food-deliver',

  projectId: 'pp98xk7d',
  dataset: 'food-delivery',

  plugins: [structureTool(), visionTool()],

  schema: {
    types: schemaTypes,
  },
})
