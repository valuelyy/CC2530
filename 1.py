import openai

client = openai.OpenAI(
    api_key="你的API_KEY",  # 替换为实际Key
    base_url="https://api.siliconflow.cn/v1"  # 官方API地址
)

# 测试调用Qwen模型
response = client.chat.completions.create(
    model="Qwen/Qwen2.5-7B-Instruct",
    messages=[{"role": "user", "content": "你好！请介绍一下你自己"}],
    max_tokens=100
)
print(response.choices[0].message.content)