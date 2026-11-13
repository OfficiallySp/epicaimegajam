import fs from 'node:fs';
const base = new URL('./', import.meta.url);
const names = ['camera','car','diamond','clapper','reel','lamp','mug','scissors','filmcan','baron'];
const results = [];
for (const name of names) {
 const path = new URL(name === 'baron' ? '../characters/baron.glb' : name+'.glb', base);
 if (!fs.existsSync(path)) continue;
 const b=fs.readFileSync(path);
 if(b.length<=204800 || b.toString('ascii',0,4)!=='glTF' || b.readUInt32LE(8)!==b.length) throw Error(name+' invalid GLB');
 const j=JSON.parse(b.toString('utf8',20,20+b.readUInt32LE(12)));
 let triangles=0; const min=[Infinity,Infinity,Infinity],max=[-Infinity,-Infinity,-Infinity];
 for(const mesh of j.meshes) for(const p of mesh.primitives) {
  if((p.mode??4)!==4) throw Error('Non-triangle primitive');
  const a=j.accessors[p.attributes.POSITION];
  triangles+=(p.indices===undefined?a.count:j.accessors[p.indices].count)/3;
  for(let k=0;k<3;k++){min[k]=Math.min(min[k],a.min[k]);max[k]=Math.max(max[k],a.max[k]);}
 }
 results.push({name,bytes:b.length,triangles,bounds:max.map((v,k)=>+(v-min[k]).toFixed(6)),nodes:j.nodes});
}
fs.writeFileSync(new URL('validation.json',base),JSON.stringify(results,null,2));
console.log(JSON.stringify(results,null,2));
