Shader "CG2023/Hatching"
{
    Properties
    {
        _Albedo("Albedo", Color) = (1,1,1,1)
        _AlbedoTexture("Albedo Texture", 2D) = "white" {}
        _Reflectance("Reflectance (Ambient, Diffuse, Specular)", Vector) = (1, 1, 1, 0)
        _SpecularExponent("Specular Exponent", Float) = 100.0

        // TODO exercise 6 - Add the required properties here
        _Thickness("Line Thickness", Float) = 0.01
        
        _Hatching0("Hatching0 Texture", 2D) = "white" {}
        _Hatching1("Hatching1 Texture", 2D) = "white" {}
        _Hatching2("Hatching2 Texture", 2D) = "white" {}
        _Hatching3("Hatching3 Texture", 2D) = "white" {}
        _Hatching4("Hatching4 Texture", 2D) = "white" {}
        _Hatching5("Hatching5 Texture", 2D) = "white" {}
    }

    SubShader
    {
        Tags { "RenderType" = "Opaque" }

        GLSLINCLUDE
        #include "UnityCG.glslinc"
        #include "ITUCG.glslinc"

        uniform vec4 _Albedo;
        uniform sampler2D _AlbedoTexture;
        uniform vec4 _AlbedoTexture_ST;
        uniform vec4 _Reflectance;
        uniform float _SpecularExponent;
        
        // TODO exercise 6 - Add the required uniforms here
        uniform float _Thickness;
        uniform sampler2D _Hatching0;
        uniform sampler2D _Hatching1;
        uniform sampler2D _Hatching2;
        uniform sampler2D _Hatching3;
        uniform sampler2D _Hatching4;
        uniform sampler2D _Hatching5;
        
        uniform vec4 _Hatching0_ST;



        // TODO exercise 6 - Compute the hatching intensity here
        float ComputeHatching(vec3 lighting, vec2 texCoords)
        {
            // TODO exercise 6.3 - Compute the lighting intensity from the lighting color luminance
            float intensity = GetColorLuminance(lighting);
            // TODO exercise 6.3 - Clamp the intensity value between 0 and 1
            intensity = saturate(intensity);

            // TODO exercise 6.3 - Multiply the intensity by the number of levels. This time the number of levels is fixed, 7, given by the number of textures + 1
            float level = intensity * 7;
            
            // TODO exercise 6.3 - Compute the blending factor, as the fractional part of the intensity
            float alpha = fract(level);
            
            // TODO exercise 6.3 - Depending on the intensity, choose up to 2 textures to sample and mix them based on the blending factor. That would be the hatching intensity
            float hatchingIntensity;

            if(level < 1)
                hatchingIntensity = mix(0.0, texture(_Hatching5, texCoords).r, alpha);
            else if(level < 2)
                hatchingIntensity = mix(texture(_Hatching5, texCoords).r, texture(_Hatching4, texCoords).r, alpha);
            else if(level < 3)
                hatchingIntensity = mix(texture(_Hatching4, texCoords).r, texture(_Hatching3, texCoords).r, alpha);
            else if(level < 4)
                hatchingIntensity = mix(texture(_Hatching3, texCoords).r, texture(_Hatching2, texCoords).r, alpha);
            else if(level < 5)
                hatchingIntensity = mix(texture(_Hatching2, texCoords).r, texture(_Hatching1, texCoords).r, alpha);
            else if(level < 6)
                hatchingIntensity = mix(texture(_Hatching1, texCoords).r, texture(_Hatching0, texCoords).r, alpha);
            else
                hatchingIntensity = mix(texture(_Hatching0, texCoords).r, 1.0, alpha);
            
            // TODO exercise 6.4 - Replace the previous step with 2 samples from the texture array. Mix them based on the blending factor to get the hatching intensity

            return hatchingIntensity.r;
            
        }
        ENDGLSL

        Pass
        {
            Name "FORWARD"
            Tags { "LightMode" = "ForwardBase" }

            GLSLPROGRAM

            struct vertexToFragment
            {
                vec3 worldPos;
                vec3 normal;
                vec4 texCoords;
            };

            #ifdef VERTEX
            out vertexToFragment v2f;

            void main()
            {
                v2f.worldPos = (unity_ObjectToWorld * gl_Vertex).xyz;
                v2f.normal = (unity_ObjectToWorld * vec4(gl_Normal, 0.0f)).xyz;
                v2f.texCoords.xy = TransformTexCoords(gl_MultiTexCoord0.xy, _AlbedoTexture_ST);

                // TODO exercise 6.3 - Transform hatching texture coordinates and pass to the fragment
                v2f.texCoords.zw = TransformTexCoords(gl_MultiTexCoord0.xy, _Hatching0_ST);

                gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
                
            }
            #endif // VERTEX

            #ifdef FRAGMENT
            in vertexToFragment v2f;

            void main()
            {
                vec3 lightDir = GetWorldSpaceLightDir(v2f.worldPos);
                vec3 viewDir = GetWorldSpaceViewDir(v2f.worldPos);

                vec3 normal = normalize(v2f.normal);

                vec3 albedo = texture(_AlbedoTexture, v2f.texCoords.xy).rgb;
                albedo *= _Albedo.rgb;

                // Like in the cel-shading exercise, we replace the albedo here with 1.0f
                vec3 lighting = BlinnPhongLighting(lightDir, viewDir, normal, vec3(1.0f), vec3(1.0f), _Reflectance.x, _Reflectance.y, _Reflectance.z, _SpecularExponent);

                float hatch = ComputeHatching(lighting, v2f.texCoords.zw);

                // Like in the cel-shading exercise, we multiply by the albedo and the light color at the end
                gl_FragColor = vec4(hatch * albedo * _LightColor0.rgb, 1.0f);
            }
            #endif // FRAGMENT

            ENDGLSL
        }
        Pass
        {
            Name "FORWARD"
            Tags { "LightMode" = "ForwardAdd" }

            ZWrite Off
            Blend One One

            GLSLPROGRAM

            struct vertexToFragment
            {
                vec3 worldPos;
                vec3 normal;
                vec4 texCoords;
            };

            #ifdef VERTEX
            out vertexToFragment v2f;

            void main()
            {
                v2f.worldPos = (unity_ObjectToWorld * gl_Vertex).xyz;
                v2f.normal = (unity_ObjectToWorld * vec4(gl_Normal, 0.0f)).xyz;
                v2f.texCoords.xy = TransformTexCoords(gl_MultiTexCoord0.xy, _AlbedoTexture_ST);

                // TODO exercise 6.3 - Transform hatching texture coordinates and pass to the fragment
                v2f.texCoords.zw = TransformTexCoords(gl_MultiTexCoord0.xy, _Hatching0_ST);

                gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
            }
            #endif // VERTEX

            #ifdef FRAGMENT
            in vertexToFragment v2f;

            void main()
            {
                vec3 lightDir = GetWorldSpaceLightDir(v2f.worldPos);
                vec3 viewDir = GetWorldSpaceViewDir(v2f.worldPos);

                vec3 normal = normalize(v2f.normal);

                vec3 albedo = texture(_AlbedoTexture, v2f.texCoords.xy).rgb;
                albedo *= _Albedo.rgb;

                // Like in the cel-shading exercise, we replace the albedo here with 1.0f. Notice that ambient reflectance is still 0.0f
                vec3 lighting = BlinnPhongLighting(lightDir, viewDir, normal, vec3(1.0f), vec3(1.0f), 0.0f, _Reflectance.y, _Reflectance.z, _SpecularExponent);

                float hatch = ComputeHatching(lighting, v2f.texCoords.zw);

                // Like in the cel-shading exercise, we multiply by the albedo and the light color at the end
                gl_FragColor = vec4(hatch * albedo * _LightColor0.rgb, 1.0f);
            }
            #endif // FRAGMENT

            ENDGLSL
        }
        Pass
        {
            Name "SHADOWCASTER"
            Tags { "LightMode" = "ShadowCaster" }

            GLSLPROGRAM

            #ifdef VERTEX
            void main()
            {
                gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
            }
            #endif // VERTEX

            #ifdef FRAGMENT
            void main()
            {
            }
            #endif // FRAGMENT

            ENDGLSL
        }
        // TODO exercise 6 - Add the outline pass here
        Pass
        {
            Name "OUTLINE"
            Tags { "LightMode" = "ForwardBase" }
            CULL Front

            GLSLPROGRAM

            #ifdef VERTEX

            void main()
            {
                vec3 worldPos = (unity_ObjectToWorld * gl_Vertex).xyz;
                vec3 worldNormal = (unity_ObjectToWorld * vec4(gl_Normal, 0.0f)).xyz;
                vec3 offsetPos = worldPos + worldNormal * _Thickness;
                gl_Position = unity_MatrixVP * vec4(offsetPos.xyz, 1.0);
        
            }
            #endif // VERTEX

            #ifdef FRAGMENT

            void main()
            {
               gl_FragColor = vec4(0.0f);
            }
            #endif // FRAGMENT

            ENDGLSL
        }
    }
}
