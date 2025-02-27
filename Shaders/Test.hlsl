
struct MeshData
{
	row_major float4x4 WVP;
	row_major float4x4 World;
};

ConstantBuffer<MeshData> Mesh : register(b0);

struct VertexData
{
	uint VertexBufferId;
};

ConstantBuffer<VertexData> VertData : register(b1);

struct Vertex
{
	float3 Position;
	float2 TexCoord;
	float3 Normal;
	float4 Tangent;
};



Vertex LoadVertex(uint Location)
{
	StructuredBuffer<Vertex> VertexBuffer = ResourceDescriptorHeap[VertData.VertexBufferId];

	Vertex output = VertexBuffer.Load(Location);
	
	return output;
}

struct VertexOutput
{
	float4 Position			: SV_Position;
	float4 WorldPosition	: WORLD_POSIITON;
	float2 TexCoord			: TEXCOORDS;
	float3 Normal			: NORMAL;
	float3 Tangent			: TANGENT;
	float3 Bitangent		: BITANGENT;
};

VertexOutput VSMain(uint VertexId : SV_VertexID)
{
	Vertex vertex = LoadVertex(VertexId);
	
	VertexOutput output;
	output.Position			= mul(Mesh.WVP, float4(vertex.Position, 1.0f));
	output.WorldPosition	= mul(Mesh.World, float4(vertex.Position, 1.0f));
	output.TexCoord			= vertex.TexCoord;
	output.Normal			= mul((float3x3)Mesh.World, vertex.Normal);
	output.Tangent			= mul((float3x3)Mesh.World, vertex.Tangent.xyz);
	output.Bitangent		= vertex.Tangent.xyz;
	
	return output;
}

float4 PSMain(VertexOutput pin) : SV_Target0
{
	return float4(pin.Normal, 1.0f);
}
