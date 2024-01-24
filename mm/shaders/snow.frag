#version 420
//Written by Scott Nykl. Default Shader for a Core 3.2+.
//Performs per fragment ambient/diffuse/specular shading with material
//properties, textures, and lights.

layout(binding = 0) uniform sampler2D TexUnit0;
// uniform sampler2D normalTexture;
// uniform sampler2D specularTexture;
layout(binding = 1) uniform sampler2D diffuseTexture;
layout(binding = 2) uniform sampler2D displacementTexture;
layout(binding = 3) uniform sampler2D groundTexture;
layout(binding = 7) uniform sampler2DShadow ShadowMap;

in vec3 currentPos;
in vec4 Color;
in vec3 VertexES;
in vec3 NormalES;
in vec2 TexCoord;
in vec4 ShadowCoord;
in vec3 vertexNormalWS;

in flat int ShadowMapShadingState;

uniform vec3 camPos;
uniform vec3 camNorm;

struct MaterialInfo {
    vec4 Ka; //Ambient
    vec4 Kd; //Diffuse
    vec4 Ks; //Specular
    float SpecularCoeff; // Specular Coefficient
};
uniform MaterialInfo Material;

layout(binding = 1, std140) uniform LightInfo {
    vec4 PosEye[8]; // Light's Eye space position (same as view space)
    vec4 Irgba[8]; // Light's Intensity for red, green, blue reflectivity components
    vec4 GlobalAmbient;
    int NumLights; // Number of lights in the LightInfo array   
} Lights;

layout(location = 0) out vec4 FragColor;

//if Lights.NumLights is zero, the alpha channel of ambient, diffuse, and specular will be 0 because
//doADS is never called since num lights is zero. As a result, when adsColor is blended with the
//its texture color, the alpha will remain zero. This will make all WOs draw invisible (this is
//probably not the best behavior for a World with no lights, we probably just want to use the global
//ambient).
//To correct this we will set the adsColor.a to be 1, in this case.
vec4 useGlobalAmbient_and_Diffuse_when_zero_lights(vec4 texColor, vec4 ambient, vec4 diffuse) {
    if(Lights.NumLights == 0) {
      //float alpha = min( texColor.a, ambient.a );
        ambient += Material.Ka + vec4(Lights.GlobalAmbient.rgb, 1);
        diffuse += Material.Kd * vec4(Lights.GlobalAmbient.rgb, 1);

        vec4 adsColor = vec4(ambient + diffuse + vec4(Lights.GlobalAmbient.rgb, 0));
        adsColor.rgba = clamp(adsColor, vec4(0, 0, 0, 0), vec4(1.0, 1.0, 1.0, 1.0));
        vec4 final = texColor * adsColor;
        return final;
    }
}

void doADS(int lightIdx, inout vec4 ambient, inout vec4 diffuse, inout vec4 specular) {
    vec3 n = normalize(NormalES);
    vec3 l = normalize(-Lights.PosEye[lightIdx].xyz); //Assume directional light, if it is spot, it will get updated next
    if(Lights.PosEye[lightIdx].w > 0.5) //if this light is a SPOT Light (it has a position), then perform spot light computations
        l = normalize(Lights.PosEye[lightIdx].xyz - VertexES); //vector to light source
    vec3 v = normalize(-VertexES); //used by spec
    vec3 r = reflect(-l, n);       //used by spec

   //Compute Ambient Contribution
    ambient += Material.Ka * Lights.Irgba[lightIdx].rgba;

   //Compute Diffuse Contribution
    float diff = clamp(dot(l, n), 0.0, 1.0);
    diffuse += Material.Kd * diff * Lights.Irgba[lightIdx].rgba;

   //Compute Specular Contribution
    float spec = pow(max(dot(r, v), 0.0), Material.SpecularCoeff);
    specular += Material.Ks * spec * Lights.Irgba[lightIdx].rgba;
}

vec4 doADS_Textures_Shadows() {
    vec4 ambient = vec4(0, 0, 0, 0);
    vec4 diffuse = vec4(0, 0, 0, 0);
    vec4 specular = vec4(0, 0, 0, 0);
    vec4 texColor = texture(diffuseTexture, TexCoord);

   //This block of code performs the same functionality as calling textureProjOffset,
   //here for reference and testing
   //float shadow = textureProj( ShadowMap, ShadowCoord );
   //vec4 shadowCoordNDC = ShadowCoord.xyzw / ShadowCoord.w; //perspective divide, bias matrix already transformed values to [0,1]   
   //float shadow = 0;
   //shadow += textureOffset( ShadowMap, shadowCoordNDC.xyz, ivec2( -1, -1 ), 0 );
   //shadow += textureOffset( ShadowMap, shadowCoordNDC.xyz, ivec2( -1,  1 ), 0 );
   //shadow += textureOffset( ShadowMap, shadowCoordNDC.xyz, ivec2(  1, -1 ), 0 );
   //shadow += textureOffset( ShadowMap, shadowCoordNDC.xyz, ivec2(  1,  1 ), 0 );
   //shadow += texture( ShadowMap, shadowCoordNDC.xyz, 0 );
   //shadow *= 0.2;

   //This code performs shadow mapping using PCF (Percent Closest Filtering)
    float shadow = 0; // Sum of the compairsons with nearby texels   
   //shadow += textureProjOffset( ShadowMap, ShadowCoord, ivec2( -1, -1 ), 0.0 );
   //shadow += textureProjOffset( ShadowMap, ShadowCoord, ivec2( 0,  -1 ), 0.0 );
   //shadow += textureProjOffset( ShadowMap, ShadowCoord, ivec2(  1,  1 ), 0.0 );
   //shadow += textureProjOffset( ShadowMap, ShadowCoord, ivec2(  -1, 0 ), 0.0 );
    shadow += textureProj(ShadowMap, ShadowCoord);
   //shadow *= 0.3333;

    for(int i = 0; i < Lights.NumLights; ++i) doADS(i, ambient, diffuse, specular);

    float alpha = min(texColor.a, ambient.a);
    vec4 adsColor = vec4(ambient + ((diffuse + specular) * shadow)) + vec4(Lights.GlobalAmbient.rgb, 0);
    adsColor = min(adsColor, vec4(1.0, 1.0, 1.0, 1.0)); //ensure textures don't get washed out (above 1.0 per channel)
    vec4 final = texColor * adsColor;
    final.a = alpha;
    if(Lights.NumLights == 0)
        final = useGlobalAmbient_and_Diffuse_when_zero_lights(texColor, ambient, diffuse); //use ambient/diffuse w/ no lights
    return final;
}

vec4 doADS_Textures(vec2 UV, float layerDepth) {
    vec4 ambient = vec4(0, 0, 0, 0);
    vec4 diffuse = vec4(0, 0, 0, 0);
    vec4 specular = vec4(0, 0, 0, 0);
    vec4 texColor = texture(diffuseTexture, UV);
    vec4 groundColor = texture(groundTexture, UV);

    if(layerDepth <= 0.02) {
        texColor = groundColor;
    }
    for(int i = 0; i < Lights.NumLights; ++i) doADS(i, ambient, diffuse, specular);

   //Let the transparency be determined by the smallest (most transparent alpha value) passed into the
   //fragment's Ambient Material property alpha OR the fragment's Texture alpha channel
    float alpha = min(texColor.a, ambient.a);
    vec4 adsColor = vec4(ambient + (diffuse + specular)) + vec4(Lights.GlobalAmbient.rgb, 0);
    adsColor = min(adsColor, vec4(1.0, 1.0, 1.0, 1.0));

    vec4 final = texColor * adsColor;
    final.a = alpha;

    if(Lights.NumLights == 0)
        final = useGlobalAmbient_and_Diffuse_when_zero_lights(texColor, ambient, diffuse); //use ambient/diffuse w/ no light

    return final;
}

//vec4 doADS_NoTextures()
//{
//   vec4 ambient  = vec4(0,0,0,0);
//   vec4 diffuse  = vec4(0,0,0,0);
//   vec4 specular = vec4(0,0,0,0);
//   
//   for( int i = 0; i < Lights.NumLights; ++i )
//      doADS( i, ambient, diffuse, specular );
//   
//   vec4 final = vec4( ambient + ( diffuse + specular ) ) + vec4( Lights.GlobalAmbient.rgb, 0 );
//   return final;
//}

vec4 doDepthBufferGenerationOnly() {
    return vec4(1, 1, 0, 1); //just need to ensure the depth buffer is populated, we don't care about the color buffer
}

void main() {
    vec4 color = vec4(0, 0, 0, 1);

    vec3 viewDirection = normalize(camPos - currentPos);

    float heightScale = 0.01;
    float minLayers = 8.0;
    float maxLayers = 64.0;

    float numLayers = mix(maxLayers, minLayers, abs(dot(vertexNormalWS, viewDirection)));
    float layerDepth = 1.0 / numLayers;
    float currentLayerDepth = 0.0;

    vec2 S = viewDirection.xy / viewDirection.z * heightScale;
    vec2 deltaUVs = S / numLayers;

    vec2 UVs = TexCoord;
    float currentDepthMapValue = 1.0 - texture(displacementTexture, UVs).r;

    while(currentLayerDepth < currentDepthMapValue) {
        UVs -= deltaUVs;
        currentDepthMapValue = 1.0 - texture(displacementTexture, UVs).r;
        currentLayerDepth += layerDepth;
    }

    vec2 prevTexCoords = UVs + deltaUVs;
    float afterDepth = currentDepthMapValue - currentLayerDepth;
    float beforeDepth = 1.0 - texture(displacementTexture, prevTexCoords).r - currentLayerDepth + layerDepth;
    float weight = afterDepth / (afterDepth - beforeDepth);
    UVs = prevTexCoords * weight + UVs * (1.0 - weight);

    if(UVs.x > 1.0 || UVs.y > 1.0 || UVs.x < 0.0 || UVs.y < 0.0)
        discard;

    color = doADS_Textures(UVs, currentLayerDepth);
// Test
    FragColor = color;
}