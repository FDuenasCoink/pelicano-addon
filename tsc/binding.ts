import { IPelicano, PelicanoOptions } from "./pelicano.interface";
import { join } from 'path';

/* eslint-disable @typescript-eslint/no-var-requires */
const addons = require('node-gyp-build')(join(__dirname, '..'));

export var Pelicano: {
  new (options: PelicanoOptions): IPelicano
} = addons.Pelicano;